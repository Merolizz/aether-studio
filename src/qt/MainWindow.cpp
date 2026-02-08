#include "MainWindow.h"
#include "ProjectModel.h"
#include "ProjectPanel.h"
#include "MonitorWidget.h"
#include "TimelineWidget.h"
#include "SettingsDialog.h"
#include "AetherRenderView.h"
#include "PageBarWidget.h"
#include "MediaPageWidget.h"
#include "PlaceholderPageWidget.h"
#include "AnimationPageWidget.h"
#include "ColorPageWidget.h"
#include "AudioPageWidget.h"
#include "DeliverPageWidget.h"
#include "HomeWidget.h"
#include "NewProjectDialog.h"
#include "aether/ProjectSettings.h"
#include "aether/ProjectFile.h"
#include "aether/PlaybackEngine.h"

#include <QByteArrayList>
#include <QToolBar>
#include <QGuiApplication>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include <QFileDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QPalette>
#include <QApplication>
#include <QFont>
#include <QMenuBar>
#include <QSplitter>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QListWidget>
#include <QKeySequence>
#include <QUrl>
#include <QFileInfo>
#include <QLabel>
#include <QStackedWidget>
#include <QKeyEvent>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QDialog>
#include <QStringConverter>
#include <QEventLoop>
#include <QTimer>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#ifdef AETHER_QT_MULTIMEDIA
#include <QMediaPlayer>
#include <QMediaMetaData>
#endif

#include "aether/LicenseManager.h"

namespace aether {

#ifdef AETHER_QT_MULTIMEDIA
struct ProbeResult { int width = 0; int height = 0; int fps = 0; qint64 durationMs = 0; };
static ProbeResult probeMediaFile(const QString& path) {
    ProbeResult r;
    QMediaPlayer player;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    auto onMeta = [&]() {
        QMediaMetaData md = player.metaData();
        if (md.contains(QMediaMetaData::Key::Resolution)) {
            QSize res = md.value(QMediaMetaData::Key::Resolution).toSize();
            r.width = res.width();
            r.height = res.height();
        }
        if (md.contains(QMediaMetaData::Key::VideoFrameRate)) {
            qreal fr = md.value(QMediaMetaData::Key::VideoFrameRate).toReal();
            r.fps = (fr > 0 && fr < 1000) ? static_cast<int>(fr + 0.5) : 0;
        }
        if (player.duration() > 0)
            r.durationMs = player.duration();
        loop.quit();
    };
    connect(&player, &QMediaPlayer::metaDataChanged, &loop, [&]() { onMeta(); });
    connect(&player, &QMediaPlayer::durationChanged, &loop, [&]() { onMeta(); });
    connect(&player, &QMediaPlayer::errorOccurred, &loop, &QEventLoop::quit);
    player.setSource(QUrl::fromLocalFile(path));
    timeout.start(3000);
    loop.exec();
    timeout.stop();
    if (r.durationMs == 0 && player.duration() > 0)
        r.durationMs = player.duration();
    if (r.width == 0 && player.metaData().contains(QMediaMetaData::Key::Resolution)) {
        QSize res = player.metaData().value(QMediaMetaData::Key::Resolution).toSize();
        r.width = res.width();
        r.height = res.height();
    }
    if (r.fps == 0 && player.metaData().contains(QMediaMetaData::Key::VideoFrameRate)) {
        qreal fr = player.metaData().value(QMediaMetaData::Key::VideoFrameRate).toReal();
        r.fps = (fr > 0 && fr < 1000) ? static_cast<int>(fr + 0.5) : 0;
    }
    return r;
}
#else
struct ProbeResult { int width = 0; int height = 0; int fps = 0; qint64 durationMs = 0; };
static ProbeResult probeMediaFile(const QString&) { return ProbeResult{}; }
#endif

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle(tr("Aether Studio"));
    setMinimumSize(1000, 650);
    resize(1400, 800);

    // QVulkanInstance: use Qt's supportedExtensions() to avoid "availableExtensions" stack corruption
    m_vulkanInstance.reset(new QVulkanInstance);
    m_vulkanInstance->setApiVersion(QVersionNumber(1, 0));
    QByteArrayList extNames;
    for (const auto& e : m_vulkanInstance->supportedExtensions())
        extNames.append(e.name);
    m_vulkanInstance->setExtensions(extNames);
    if (!m_vulkanInstance->create()) {
        QMessageBox::warning(this, tr("Vulkan"),
            tr("QVulkanInstance creation failed. Vulkan rendering may be disabled.\n%1")
                .arg(m_vulkanInstance->errorCode()));
    }
    // In Qt6 setVulkanInstance is on QWindow; set on render view when it is created (e.g. in MonitorWidget)

    m_projectModel.reset(new ProjectModel(this));
    setupDarkTheme();
    setupMenuBar();
    setupToolsToolbar();
    setupPanels();
    setupCentralArea();
    setupPageNavigation();
    setupLicenseStatus();
    enterHomeState();
}

MainWindow::~MainWindow() {
    m_renderView = nullptr;
    m_renderContainer = nullptr;
}

void MainWindow::setupDarkTheme() {
    QFont appFont(QStringLiteral("Segoe UI"), 10);
    if (!appFont.exactMatch())
        appFont = QFont(QStringLiteral("Inter"), 10);
    if (!appFont.exactMatch())
        appFont = QApplication::font();
    QApplication::setFont(appFont);

    QPalette p = palette();
    p.setColor(QPalette::Window, QColor(0x12, 0x12, 0x12));
    p.setColor(QPalette::WindowText, QColor(230, 230, 230));
    p.setColor(QPalette::Base, QColor(0x1E, 0x1E, 0x1E));
    p.setColor(QPalette::AlternateBase, QColor(0x25, 0x25, 0x26));
    p.setColor(QPalette::Text, QColor(230, 230, 230));
    p.setColor(QPalette::Button, QColor(0x2d, 0x2d, 0x32));
    p.setColor(QPalette::ButtonText, QColor(230, 230, 230));
    p.setColor(QPalette::Highlight, QColor(94, 129, 172));
    p.setColor(QPalette::HighlightedText, Qt::white);
    setPalette(p);
    setStyleSheet(
        "QMainWindow, QWidget { background: #121212; }"
        "QToolBar { background: #1E1E1E; border: none; border-bottom: 1px solid #333333; spacing: 4px; padding: 2px; }"
        "QToolBar QToolButton { background: #2d2d32; color: #e0e0e0; border: 1px solid #333333; border-radius: 4px; padding: 4px 8px; }"
        "QToolBar QToolButton:hover { background: #3c3c3c; }"
        "QToolBar QToolButton:checked { background: #0e639c; border-color: #1177bb; }"
        "QDockWidget { titlebar-close-icon: none; titlebar-normal-icon: none; background: #121212; }"
        "QDockWidget::title { background: #1E1E1E; color: #e0e0e0; padding: 6px 8px; border: 1px solid #333333; border-radius: 4px; }"
        "QPushButton { background: #2d2d32; color: #e0e0e0; border: 1px solid #333333; border-radius: 4px; padding: 6px 12px; }"
        "QPushButton:hover { background: #3c3c3c; }"
        "QPushButton:pressed { background: #252526; }"
        "QLineEdit, QComboBox, QSpinBox { background: #1E1E1E; color: #e0e0e0; border: 1px solid #333333; border-radius: 4px; padding: 4px; min-height: 20px; }"
        "QListWidget { background: #1E1E1E; border: 1px solid #333333; border-radius: 4px; }"
        "QListWidget::item { padding: 4px 8px; }"
        "QListWidget::item:selected { background: #2d5a87; }"
        "QSplitter::handle { width: 6px; background: #333333; border-radius: 2px; }"
        "QSplitter::handle:hover { background: #444444; }"
        "QMenuBar { background: #1E1E1E; color: #e0e0e0; border-bottom: 1px solid #333333; }"
        "QMenuBar::item:selected { background: #2d2d32; border-radius: 4px; }"
        "QMenu { background: #1E1E1E; border: 1px solid #333333; border-radius: 4px; }"
        "QStatusBar { background: #1E1E1E; color: #888; border-top: 1px solid #333333; }"
    );
}

void MainWindow::setupMenuBar() {
    QMenu* fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(tr("New project"), QKeySequence::New, this, &MainWindow::onNewProject);
    fileMenu->addAction(tr("Open project..."), QKeySequence::Open, this, [this]() {
        QString path = QFileDialog::getOpenFileName(this, tr("Open Project"), QString(),
            tr("Aether Project (*.aether);;All Files (*)"));
        if (!path.isEmpty()) onOpenProjectPath(path);
    });
    fileMenu->addAction(tr("Import media..."), QKeySequence(Qt::CTRL | Qt::Key_I), this, &MainWindow::onImportMedia);
    fileMenu->addAction(tr("Export..."), QKeySequence(Qt::CTRL | Qt::Key_M), this, &MainWindow::onExport);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Settings..."), this, &MainWindow::onSettings);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Exit"), QKeySequence::Quit, this, &QWidget::close);

    QMenu* editMenu = menuBar()->addMenu(tr("Edit"));
    editMenu->addAction(tr("Undo"), QKeySequence::Undo, this, &MainWindow::onUndo);
    editMenu->addAction(tr("Redo"), QKeySequence::Redo, this, &MainWindow::onRedo);
    editMenu->addSeparator();
    editMenu->addAction(tr("Cut"), QKeySequence::Cut, this, &MainWindow::onCut);
    editMenu->addAction(tr("Copy"), QKeySequence::Copy, this, &MainWindow::onCopy);
    editMenu->addAction(tr("Paste"), QKeySequence::Paste, this, &MainWindow::onPaste);
    editMenu->addSeparator();
    editMenu->addAction(tr("Preferences..."), QKeySequence::Preferences, this, &MainWindow::onSettings);

    QMenu* clipMenu = menuBar()->addMenu(tr("Clip"));
    clipMenu->addAction(tr("Enable"), this, &MainWindow::onClipEnable);
    clipMenu->addAction(tr("Link/Unlink"), this, &MainWindow::onClipLinkUnlink);
    clipMenu->addSeparator();
    clipMenu->addAction(tr("Video Options"), this, &MainWindow::onClipVideoOptions);
    clipMenu->addAction(tr("Audio Options"), this, &MainWindow::onClipAudioOptions);

    QMenu* sequenceMenu = menuBar()->addMenu(tr("Sequence"));
    sequenceMenu->addAction(tr("Add Track"), this, &MainWindow::onSequenceAddTrack);
    sequenceMenu->addAction(tr("Delete Track"), this, &MainWindow::onSequenceDeleteTrack);
    sequenceMenu->addSeparator();
    sequenceMenu->addAction(tr("Render In to Out"), this, &MainWindow::onSequenceRenderInToOut);

    QMenu* markersMenu = menuBar()->addMenu(tr("Markers"));
    markersMenu->addAction(tr("Add Marker"), QKeySequence(Qt::Key_M), this, &MainWindow::onAddMarker);
    markersMenu->addAction(tr("Go to Next Marker"), this, &MainWindow::onNextMarker);
    markersMenu->addAction(tr("Go to Previous Marker"), this, &MainWindow::onPrevMarker);

    QMenu* windowMenu = menuBar()->addMenu(tr("Window"));
    windowMenu->addAction(tr("Project"), this, &MainWindow::onShowPanelProject);
    windowMenu->addAction(tr("Properties"), this, &MainWindow::onShowPanelProperties);
    windowMenu->addAction(tr("Effects"), this, &MainWindow::onShowPanelEffects);
    windowMenu->addAction(tr("Timeline"), this, &MainWindow::onShowPanelTimeline);

    QMenu* helpMenu = menuBar()->addMenu(tr("Help"));
    helpMenu->addAction(tr("About Aether Studio"), []() {});
}

void MainWindow::setupToolsToolbar() {
    m_toolsToolbar = new QToolBar(tr("Tools"), this);
    m_toolsToolbar->setObjectName("toolsToolbar");
    m_toolsToolbar->setMovable(false);
    m_toolsToolbar->setFloatable(false);
    m_toolsToolbar->setIconSize(QSize(24, 24));

    QActionGroup* toolGroup = new QActionGroup(this);
    toolGroup->setExclusive(true);

    auto addTool = [this, toolGroup](const QString& name, const QVariant& toolId, const QKeySequence& shortcut) {
        QAction* a = m_toolsToolbar->addAction(name);
        a->setCheckable(true);
        a->setData(toolId);
        a->setShortcut(shortcut);
        toolGroup->addAction(a);
        if (toolId.toInt() == 0) a->setChecked(true);
        return a;
    };

    addTool(tr("Selection (V)"), static_cast<int>(TimelineWidget::Tool::Selection), QKeySequence(Qt::Key_V));
    addTool(tr("Razor (C)"), static_cast<int>(TimelineWidget::Tool::Razor), QKeySequence(Qt::Key_C));
    addTool(tr("Slip"), static_cast<int>(TimelineWidget::Tool::Slip), QKeySequence(Qt::Key_Y));
    addTool(tr("Slide"), static_cast<int>(TimelineWidget::Tool::Slide), QKeySequence(Qt::Key_U));
    addTool(tr("Pen"), static_cast<int>(TimelineWidget::Tool::Pen), QKeySequence(Qt::Key_P));
    addTool(tr("Hand (H)"), static_cast<int>(TimelineWidget::Tool::Hand), QKeySequence(Qt::Key_H));
    addTool(tr("Zoom (Z)"), static_cast<int>(TimelineWidget::Tool::Zoom), QKeySequence(Qt::Key_Z));

    connect(toolGroup, &QActionGroup::triggered, this, &MainWindow::onToolTriggered);
    addToolBar(Qt::TopToolBarArea, m_toolsToolbar);
}

void MainWindow::setupPanels() {
    m_projectPanel = new ProjectPanel(m_projectModel.get(), this);
    m_projectPanel->setObjectName("ProjectPanel");
    connect(m_projectPanel, &ProjectPanel::importRequested, this, &MainWindow::doImportMedia);
    connect(m_projectPanel, &ProjectPanel::filesDropped, this, &MainWindow::doImportMediaPaths);
    connect(m_projectPanel, &ProjectPanel::addToTimelineRequested, this, &MainWindow::onAddToTimeline);
    connect(m_projectPanel, &ProjectPanel::createProxyRequested, this, [this](const QString& path) {
        statusBar()->showMessage(tr("Create proxy for: %1 (use Settings to set proxy folder)").arg(QFileInfo(path).fileName()), 4000);
    });

    m_projectDock = new QDockWidget(tr("Project"), this);
    m_projectDock->setObjectName("ProjectDock");
    m_projectDock->setWidget(m_projectPanel);
    m_projectDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, m_projectDock);

    m_propertiesStack = new QStackedWidget(this);
    const char* propLabels[] = { "Media", "Edit", "Animation", "Color", "Audio", "Deliver" };
    for (int i = 0; i < 6; i++) {
        if (i == 1) {
            m_editPropertiesStack = new QStackedWidget(this);
            QLabel* noSel = new QLabel(tr("No clip selected.\nSelect a clip on the timeline."), this);
            noSel->setStyleSheet("color: #888; padding: 12px;");
            noSel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            m_editPropertiesStack->addWidget(noSel);
            QWidget* videoProps = new QWidget(this);
            QFormLayout* vfl = new QFormLayout(videoProps);
            vfl->addRow(tr("Scale:"), new QLabel(tr("100%")));
            vfl->addRow(tr("Speed:"), new QLabel(tr("100%")));
            vfl->addRow(tr("Opacity:"), new QLabel(tr("100%")));
            videoProps->setStyleSheet("color: #888;");
            m_editPropertiesStack->addWidget(videoProps);
            QWidget* audioProps = new QWidget(this);
            QFormLayout* afl = new QFormLayout(audioProps);
            afl->addRow(tr("Volume:"), new QLabel(tr("0 dB")));
            afl->addRow(tr("Pan:"), new QLabel(tr("Center")));
            audioProps->setStyleSheet("color: #888;");
            m_editPropertiesStack->addWidget(audioProps);
            QWidget* photoProps = new QWidget(this);
            QFormLayout* pfl = new QFormLayout(photoProps);
            pfl->addRow(tr("Duration:"), new QLabel(tr("—")));
            pfl->addRow(tr("Scale:"), new QLabel(tr("100%")));
            photoProps->setStyleSheet("color: #888;");
            m_editPropertiesStack->addWidget(photoProps);
            m_propertiesStack->addWidget(m_editPropertiesStack);
        } else {
            QLabel* pl = new QLabel(tr("Properties — %1").arg(tr(propLabels[i])), this);
            pl->setStyleSheet("color: #888; padding: 12px;");
            pl->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            m_propertiesStack->addWidget(pl);
        }
    }
    m_propertiesDock = new QDockWidget(tr("Properties"), this);
    m_propertiesDock->setObjectName("PropertiesDock");
    m_propertiesDock->setWidget(m_propertiesStack);
    m_propertiesDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);

    m_effectsStack = new QStackedWidget(this);
    const char* effectLabels[] = { "Media effects", "Clip effects", "Node effects", "LUT / Wheels", "Channel effects", "Output effects" };
    for (int i = 0; i < 6; i++) {
        if (i == 1) {
            m_editEffectsStack = new QStackedWidget(this);
            QLabel* noEff = new QLabel(tr("No clip selected.\nSelect a clip to see effects."), this);
            noEff->setStyleSheet("color: #888; padding: 12px;");
            noEff->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            m_editEffectsStack->addWidget(noEff);
            QListWidget* videoEff = new QListWidget(this);
            videoEff->setStyleSheet("QListWidget { background: #1e1e1e; border: none; } QListWidget::item { padding: 4px 8px; }");
            for (const QString& name : { tr("Opacity"), tr("Scale"), tr("Position"), tr("Rotation"), tr("Blur"), tr("Brightness & Contrast"), tr("Color Balance") })
                videoEff->addItem(name);
            m_editEffectsStack->addWidget(videoEff);
            QListWidget* audioEff = new QListWidget(this);
            audioEff->setStyleSheet("QListWidget { background: #1e1e1e; border: none; } QListWidget::item { padding: 4px 8px; }");
            for (const QString& name : { tr("Volume"), tr("EQ"), tr("Compressor"), tr("Reverb") })
                audioEff->addItem(name);
            m_editEffectsStack->addWidget(audioEff);
            QListWidget* photoEff = new QListWidget(this);
            photoEff->setStyleSheet("QListWidget { background: #1e1e1e; border: none; } QListWidget::item { padding: 4px 8px; }");
            photoEff->addItem(tr("Scale"));
            photoEff->addItem(tr("Opacity"));
            m_editEffectsStack->addWidget(photoEff);
            m_effectsStack->addWidget(m_editEffectsStack);
        } else {
            QListWidget* el = new QListWidget(this);
            el->setStyleSheet("QListWidget { background: #1e1e1e; border: none; } QListWidget::item { padding: 4px 8px; }");
            el->addItem(tr(effectLabels[i]));
            m_effectsStack->addWidget(el);
        }
    }

    m_effectsDock = new QDockWidget(tr("Effects"), this);
    m_effectsDock->setObjectName("EffectsDock");
    m_effectsDock->setWidget(m_effectsStack);
    m_effectsDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, m_effectsDock);
}

void MainWindow::setupCentralArea() {
    m_monitor = new MonitorWidget(this);
    m_playbackEngine.reset(new PlaybackEngine(this));
    m_monitor->setPlaybackEngine(m_playbackEngine.get());
    m_timeline = new TimelineWidget(m_projectModel.get(), this);
    connect(m_timeline, &TimelineWidget::playheadMoved, this, &MainWindow::onPlayheadMoved);
    connect(m_timeline, &TimelineWidget::clipSelected, this, &MainWindow::onClipSelected);
    connect(m_timeline, &TimelineWidget::addEffectRequested, this, [this](int trackIndex, int clipIndex) {
        Q_UNUSED(trackIndex);
        Q_UNUSED(clipIndex);
        if (m_effectsDock) m_effectsDock->show();
        statusBar()->showMessage(tr("Add effect to clip — use Effects panel"), 3000);
    });
    connect(m_timeline, &TimelineWidget::interpretFootageRequested, this, &MainWindow::onInterpretFootageRequested);
    connect(m_playbackEngine.get(), &PlaybackEngine::positionChanged, this, &MainWindow::onPlaybackPositionChanged);
    connect(m_monitor, &MonitorWidget::playbackError, this, [this](const QString& msg) {
        statusBar()->showMessage(tr("Playback error: %1").arg(msg), 8000);
    });

    m_centralSplitter = new QSplitter(Qt::Vertical, this);
    m_centralSplitter->addWidget(m_monitor);
    m_centralSplitter->addWidget(m_timeline);
    m_centralSplitter->setStretchFactor(0, 1);
    m_centralSplitter->setStretchFactor(1, 0);
    m_centralSplitter->setSizes({ 400, 320 });

    m_stackedPages = new QStackedWidget(this);
    HomeWidget* homePage = new HomeWidget(this);
    connect(homePage, &HomeWidget::newProjectRequested, this, &MainWindow::onNewProject);
    connect(homePage, &HomeWidget::openRecentRequested, this, &MainWindow::onOpenProjectPath);
    m_stackedPages->addWidget(homePage);
    m_mediaPage = new MediaPageWidget(m_projectModel.get(), this);
    connect(m_mediaPage, &MediaPageWidget::importRequested, this, &MainWindow::doImportMedia);
    connect(m_mediaPage, &MediaPageWidget::addToTimelineRequested, this, &MainWindow::onAddToTimeline);
    m_stackedPages->addWidget(m_mediaPage);
    m_stackedPages->addWidget(m_centralSplitter);
    m_stackedPages->addWidget(new AnimationPageWidget(this));
    m_stackedPages->addWidget(new ColorPageWidget(this));
    m_stackedPages->addWidget(new AudioPageWidget(this));
    m_stackedPages->addWidget(new DeliverPageWidget(this));

    m_pageBar = new PageBarWidget(this);
    connect(m_pageBar, &PageBarWidget::pageChanged, this, &MainWindow::onPageChanged);

    QWidget* centralContainer = new QWidget(this);
    QVBoxLayout* centralLayout = new QVBoxLayout(centralContainer);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);
    centralLayout->addWidget(m_stackedPages, 1);
    centralLayout->addWidget(m_pageBar);

    setCentralWidget(centralContainer);
}

void MainWindow::enterHomeState() {
    m_appState = AppState::Home;
    m_stackedPages->setCurrentIndex(0);
    m_currentPage = 0;
    if (m_pageBar) m_pageBar->hide();
    if (m_projectDock) m_projectDock->hide();
    if (m_propertiesDock) m_propertiesDock->hide();
    if (m_effectsDock) m_effectsDock->hide();
    if (m_toolsToolbar) m_toolsToolbar->hide();
}

void MainWindow::enterProjectState() {
    m_appState = AppState::Project;
    if (m_pageBar) m_pageBar->show();
    if (m_projectDock) m_projectDock->show();
    if (m_propertiesDock) m_propertiesDock->show();
    if (m_effectsDock) m_effectsDock->show();
    m_stackedPages->setCurrentIndex(2);
    m_currentPage = 2;
    if (m_pageBar) m_pageBar->setCurrentPage(1);
    if (m_propertiesStack) m_propertiesStack->setCurrentIndex(1);
    if (m_effectsStack) m_effectsStack->setCurrentIndex(1);
    if (m_toolsToolbar) m_toolsToolbar->setVisible(true);
    if (m_timeline) m_timeline->setFocus();
    if (m_monitor) m_monitor->setProjectFps(m_projectSettings.fps);
}

void MainWindow::setupPageNavigation() {
    connect(m_stackedPages, &QStackedWidget::currentChanged, this, [this](int index) {
        m_currentPage = index;
        if (m_pageBar && index >= 1 && index <= PageBarWidget::PageCount)
            m_pageBar->setCurrentPage(index - 1);
    });
}

void MainWindow::onPageChanged(int index) {
    if (index < 0 || index >= PageBarWidget::PageCount) return;
    int stackIndex = index + 1;
    m_stackedPages->setCurrentIndex(stackIndex);
    m_currentPage = stackIndex;
    if (m_propertiesStack) m_propertiesStack->setCurrentIndex(index);
    if (m_effectsStack) m_effectsStack->setCurrentIndex(index);
    if (index == 1)
        refreshEditPanelsForClipType();
    if (index != 1 && m_monitor)
        m_monitor->pause();
    if (m_toolsToolbar)
        m_toolsToolbar->setVisible(stackIndex == 2);
    if (stackIndex == 2 && m_timeline)
        m_timeline->setFocus();
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (m_currentPage == 2 && m_monitor) {
        int k = event->key();
        if (k == Qt::Key_J || k == Qt::Key_K || k == Qt::Key_L) {
            m_monitor->onJKL(k);
            event->accept();
            return;
        }
    }
    QMainWindow::keyPressEvent(event);
}

static QString readIniValue(const QString& path, const QString& key) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
    QTextStream in(&f);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        int eq = line.indexOf(QLatin1Char('='));
        if (eq <= 0) continue;
        if (line.left(eq).trimmed() == key)
            return line.mid(eq + 1).trimmed();
    }
    return QString();
}

QString MainWindow::resolveProxyPath(const QString& sourcePath) const {
    QString path = QApplication::applicationDirPath() + QLatin1String("/settings.ini");
    if (readIniValue(path, QStringLiteral("UseProxyForPlayback")) != QLatin1String("1"))
        return QString();
    QString proxyDir = readIniValue(path, QStringLiteral("ProxyDir"));
    if (proxyDir.isEmpty()) proxyDir = QStringLiteral("./cache/proxies");
    QString res = readIniValue(path, QStringLiteral("ProxyResolution"));
    if (res.isEmpty()) res = QStringLiteral("full");
    QString suffix = QStringLiteral("_proxy_med");
    if (res == QLatin1String("half")) suffix = QStringLiteral("_proxy_half");
    else if (res == QLatin1String("quarter")) suffix = QStringLiteral("_proxy_quarter");
    QFileInfo fi(sourcePath);
    QString stem = fi.completeBaseName();
    QString ext = fi.suffix();
    if (ext.isEmpty()) ext = QStringLiteral("mp4");
    QString proxyPath = proxyDir + QLatin1Char('/') + stem + suffix + QLatin1Char('.') + ext;
    if (QFileInfo::exists(proxyPath))
        return proxyPath;
    if (suffix != QLatin1String("_proxy_med")) {
        proxyPath = proxyDir + QLatin1Char('/') + stem + QStringLiteral("_proxy_med.") + ext;
        if (QFileInfo::exists(proxyPath)) return proxyPath;
    }
    return QString();
}

void MainWindow::setupLicenseStatus() {
    aether::LicenseManager::getInstance().initialize();
    std::string hwid = aether::LicenseManager::getInstance().getCurrentHWID();
    if (!hwid.empty())
        statusBar()->showMessage(QString("HWID: %1").arg(QString::fromStdString(hwid)), 5000);
    else
        statusBar()->showMessage(tr("Ready"), 5000);
}

void MainWindow::doImportMedia() {
    QStringList paths = QFileDialog::getOpenFileNames(this, tr("Import Media"),
        QString(), tr("Media (*.mp4 *.mov *.avi *.mkv *.webm *.mp3 *.wav *.png *.jpg *.jpeg);;All (*.*)"));
    if (paths.isEmpty()) return;
    doImportMediaPaths(paths);
}

void MainWindow::doImportMediaPaths(const QStringList& paths) {
    if (paths.isEmpty()) return;
    int n = 0;
    for (const QString& path : paths) {
        QFileInfo fi(path);
        if (!fi.exists()) continue;
        m_projectModel->addMedia(path, fi.fileName(), 0, true, path.endsWith(QStringLiteral(".mp3"), Qt::CaseInsensitive) || path.endsWith(QStringLiteral(".wav"), Qt::CaseInsensitive) ? false : true);
        n++;
    }
    if (n > 0) statusBar()->showMessage(tr("Imported %1 file(s)").arg(n), 3000);
}

void MainWindow::onImportMedia() {
    doImportMedia();
}

void MainWindow::onAddToTimeline(const QString& mediaPath) {
    const auto& tracks = m_projectModel->tracks();
    int totalClips = 0;
    for (const Track& t : tracks)
        totalClips += t.clips.size();
    bool isFirstClip = (totalClips == 0);

    qint64 startMs = 0;
    for (const Track& t : tracks) {
        for (const TimelineClip& c : t.clips) {
            qint64 end = c.timelineStartMs + (c.sourceOutMs - c.sourceInMs);
            if (end > startMs) startMs = end;
        }
    }
    qint64 durationMs = 60000;
    bool isVideo = true;
    for (const MediaItem& m : m_projectModel->media()) {
        if (m.path == mediaPath) {
            durationMs = m.durationMs > 0 ? m.durationMs : 60000;
            isVideo = m.isVideo;
            break;
        }
    }

    if (isVideo) {
        const MediaItem* item = nullptr;
        for (const MediaItem& m : m_projectModel->media()) {
            if (m.path == mediaPath) { item = &m; break; }
        }
        if (item && (item->width <= 0 || item->height <= 0)) {
            ProbeResult r = probeMediaFile(mediaPath);
            m_projectModel->setMediaMetadataByPath(mediaPath, r.width, r.height, r.fps);
            if (r.durationMs > 0)
                durationMs = r.durationMs;
        }
    }

    int videoTrack = 0;
    for (int i = 0; i < tracks.size(); i++) {
        if (tracks[i].isVideo) { videoTrack = i; break; }
    }
    m_projectModel->addClipToTrack(videoTrack, mediaPath, 0, durationMs, startMs);

    if (isFirstClip && isVideo) {
        QMessageBox::StandardButton btn = QMessageBox::question(this, tr("Match sequence settings"),
            tr("Match sequence settings to this clip?\n(Resolution and frame rate will be set from the clip.)"),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if (btn == QMessageBox::Yes) {
            int w = 1920, h = 1080, f = 30;
            for (const MediaItem& m : m_projectModel->media()) {
                if (m.path == mediaPath) {
                    if (m.width > 0 && m.height > 0) { w = m.width; h = m.height; }
                    if (m.fps > 0 && m.fps <= 120) f = m.fps;
                    break;
                }
            }
            m_projectSettings.width = w;
            m_projectSettings.height = h;
            m_projectSettings.fps = f;
            if (m_monitor) m_monitor->setProjectFps(f);
            statusBar()->showMessage(tr("Sequence settings matched to clip (%1×%2, %3 fps)").arg(w).arg(h).arg(f), 3000);
        }
    }

    m_timeline->update();
    statusBar()->showMessage(tr("Added to timeline"), 2000);
}

void MainWindow::onInterpretFootageRequested(const QString& mediaPath) {
    if (!m_projectModel) return;
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Interpret Footage"));
    QFormLayout* form = new QFormLayout(&dlg);
    QSpinBox* fpsSpin = new QSpinBox(&dlg);
    fpsSpin->setRange(0, 120);
    fpsSpin->setSpecialValueText(tr("Use file default"));
    fpsSpin->setValue(m_projectModel->mediaInterpretFps(mediaPath));
    form->addRow(tr("Assume frame rate (fps):"), fpsSpin);
    QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    form->addRow(box);
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() == QDialog::Accepted)
        m_projectModel->setMediaInterpretFpsByPath(mediaPath, fpsSpin->value());
}

MainWindow::EditClipType MainWindow::selectedClipType() const {
    if (!m_projectModel || m_lastSelectedTrack < 0 || m_lastSelectedClip < 0) return EditClipType::None;
    const auto& tracks = m_projectModel->tracks();
    if (m_lastSelectedTrack >= tracks.size() || m_lastSelectedClip >= tracks[m_lastSelectedTrack].clips.size())
        return EditClipType::None;
    QString path = tracks[m_lastSelectedTrack].clips[m_lastSelectedClip].mediaPath;
    for (const MediaItem& m : m_projectModel->media()) {
        if (m.path == path) {
            if (m.isAudio) return EditClipType::Audio;
            if (m.isVideo) {
                QString lower = path.toLower();
                if (lower.endsWith(QLatin1String(".jpg")) || lower.endsWith(QLatin1String(".jpeg")) ||
                    lower.endsWith(QLatin1String(".png")) || lower.endsWith(QLatin1String(".bmp")) ||
                    lower.endsWith(QLatin1String(".gif")) || lower.endsWith(QLatin1String(".webp")))
                    return EditClipType::Photo;
                return EditClipType::Video;
            }
            return EditClipType::Video;
        }
    }
    if (path.toLower().endsWith(QLatin1String(".mp3")) || path.toLower().endsWith(QLatin1String(".wav")))
        return EditClipType::Audio;
    return EditClipType::Video;
}

void MainWindow::refreshEditPanelsForClipType() {
    int idx = static_cast<int>(selectedClipType());
    if (m_editPropertiesStack && m_editPropertiesStack->count() > idx)
        m_editPropertiesStack->setCurrentIndex(idx);
    if (m_editEffectsStack && m_editEffectsStack->count() > idx)
        m_editEffectsStack->setCurrentIndex(idx);
}

void MainWindow::onPlayheadMoved(qint64 ms) {
    m_monitor->setTimecodeFromSequence(ms);
    updateMonitorForPlayhead();
}

void MainWindow::onPlaybackPositionChanged(qint64 engineMs) {
    if (!m_timeline || m_currentMonitorClipTimelineStartMs < 0) return;
    double ratio = m_currentMonitorClipSpeedRatio > 0.001 ? m_currentMonitorClipSpeedRatio : 1.0;
    qint64 sequenceMs = m_currentMonitorClipTimelineStartMs
        + static_cast<qint64>((engineMs - m_currentMonitorClipSourceInMs) / ratio);
    if (sequenceMs < 0) sequenceMs = 0;
    m_timeline->setPlayheadPositionMs(sequenceMs);
}

void MainWindow::updateMonitorForPlayhead() {
    qint64 ms = m_timeline->playheadPositionMs();
    const auto& tracks = m_projectModel->tracks();
    m_currentMonitorClipTimelineStartMs = -1;
    for (const Track& t : tracks) {
        if (!t.isVideo) continue;
        for (const TimelineClip& c : t.clips) {
            qint64 span = (c.speedRatio > 0.001)
                ? static_cast<qint64>((c.sourceOutMs - c.sourceInMs) / c.speedRatio)
                : (c.sourceOutMs - c.sourceInMs);
            qint64 end = c.timelineStartMs + span;
            if (ms >= c.timelineStartMs && ms < end) {
                qint64 posInClip = c.sourceInMs + static_cast<qint64>((ms - c.timelineStartMs) * c.speedRatio);
                QString pathToUse = resolveProxyPath(c.mediaPath);
                m_monitor->setSource(pathToUse.isEmpty() ? c.mediaPath : pathToUse);
                m_monitor->setPositionMs(posInClip);
                m_currentMonitorClipTimelineStartMs = c.timelineStartMs;
                m_currentMonitorClipSourceInMs = c.sourceInMs;
                m_currentMonitorClipSpeedRatio = c.speedRatio;
                return;
            }
        }
    }
    m_monitor->setPositionMs(ms);
}

void MainWindow::onClipSelected(int trackIndex, int clipIndex) {
    if (!m_projectModel || trackIndex < 0 || clipIndex < 0) return;
    const auto& tracks = m_projectModel->tracks();
    if (trackIndex >= tracks.size() || clipIndex >= tracks[trackIndex].clips.size()) return;
    m_lastSelectedTrack = trackIndex;
    m_lastSelectedClip = clipIndex;
    const TimelineClip& c = tracks[trackIndex].clips[clipIndex];
    QString pathToUse = resolveProxyPath(c.mediaPath);
    m_monitor->setSource(pathToUse.isEmpty() ? c.mediaPath : pathToUse);
    m_monitor->setPositionMs(c.sourceInMs);
    m_currentMonitorClipTimelineStartMs = c.timelineStartMs;
    m_currentMonitorClipSourceInMs = c.sourceInMs;
    m_currentMonitorClipSpeedRatio = c.speedRatio;
    refreshEditPanelsForClipType();
    statusBar()->showMessage(tr("Selected clip: %1").arg(QFileInfo(c.mediaPath).fileName()), 2000);
}

void MainWindow::onToolTriggered() {
    QAction* a = qobject_cast<QAction*>(sender());
    if (!a || !m_timeline) return;
    QVariant v = a->data();
    if (v.canConvert<int>())
        m_timeline->setTool(static_cast<TimelineWidget::Tool>(v.toInt()));
}

void MainWindow::onShowPanelProject() {
    if (m_projectDock) m_projectDock->show();
}

void MainWindow::onShowPanelProperties() {
    if (m_propertiesDock) m_propertiesDock->show();
}

void MainWindow::onShowPanelEffects() {
    if (m_effectsDock) m_effectsDock->show();
}

void MainWindow::onShowPanelTimeline() {
    if (m_centralSplitter) m_centralSplitter->setFocus();
}

void MainWindow::onSettings() {
    SettingsDialog dlg(this);
    dlg.exec();
}

void MainWindow::onNewProject() {
    NewProjectDialog dlg(this);
    dlg.setSettings(m_projectSettings);
    if (dlg.exec() != QDialog::Accepted)
        return;
    m_projectSettings = dlg.settings();
    QString name = dlg.projectName();
    QString location = dlg.saveLocation();
    if (location.isEmpty()) return;
    QString safeName = name;
    for (QChar& c : safeName) {
        if (c == QLatin1Char('/') || c == QLatin1Char('\\') || c == QLatin1Char(':'))
            c = QLatin1Char('_');
    }
    if (safeName.isEmpty()) safeName = QStringLiteral("Untitled Project");
    if (!safeName.endsWith(QLatin1String(".aether")))
        safeName += QLatin1String(".aether");
    QString projectPath = location + QLatin1Char('/') + safeName;
    if (!ProjectFile::saveToPath(projectPath, dlg.projectName(), location, m_projectSettings)) {
        QMessageBox::critical(this, tr("New Project"), tr("Could not create project file at:\n%1").arg(projectPath));
        return;
    }
    m_currentProjectPath = projectPath;
    appendToRecentProjects(projectPath);
    m_projectModel->clearMedia();
    m_projectModel->clearAllClips();
    m_timeline->setPlayheadPositionMs(0);
    m_monitor->clearSource();
    if (m_timeline) m_timeline->update();
    enterProjectState();
    statusBar()->showMessage(tr("Project created: %1").arg(projectPath), 3000);
}

void MainWindow::onOpenProjectPath(const QString& path) {
    if (path.isEmpty() || !QFileInfo::exists(path)) {
        QMessageBox::warning(this, tr("Open Project"), tr("File not found: %1").arg(path));
        return;
    }
    QString name, location;
    ProjectSettings loaded;
    if (!ProjectFile::loadFromPath(path, &name, &location, &loaded)) {
        QMessageBox::critical(this, tr("Open Project"), tr("Could not load project file."));
        return;
    }
    m_projectSettings = loaded;
    m_currentProjectPath = path;
    appendToRecentProjects(path);
    m_projectModel->clearMedia();
    m_projectModel->clearAllClips();
    m_timeline->setPlayheadPositionMs(0);
    m_monitor->clearSource();
    if (m_timeline) m_timeline->update();
    enterProjectState();
    statusBar()->showMessage(tr("Opened project: %1").arg(path), 3000);
}

void MainWindow::appendToRecentProjects(const QString& path) {
    if (path.isEmpty()) return;
    QString filePath = QApplication::applicationDirPath() + QLatin1String("/recent_projects.txt");
    QFile f(filePath);
    QByteArray toPrepend = (path + QLatin1Char('\n')).toUtf8();
    QByteArray existing;
    if (f.open(QIODevice::ReadOnly))
        existing = f.readAll();
    f.close();
    QStringList lines = QString::fromUtf8(existing).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    lines.removeAll(path);
    lines.prepend(path);
    while (lines.size() > 10) lines.removeLast();
    if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&f);
        out.setEncoding(QStringConverter::Utf8);
        for (const QString& line : lines)
            out << line << "\n";
    }
}

void MainWindow::onUndo() {
    statusBar()->showMessage(tr("Undo"), 2000);
}

void MainWindow::onRedo() {
    statusBar()->showMessage(tr("Redo"), 2000);
}

void MainWindow::onCut() {
    statusBar()->showMessage(tr("Cut"), 2000);
}

void MainWindow::onCopy() {
    statusBar()->showMessage(tr("Copy"), 2000);
}

void MainWindow::onPaste() {
    statusBar()->showMessage(tr("Paste"), 2000);
}

void MainWindow::onClipEnable() {
    statusBar()->showMessage(tr("Clip Enable"), 2000);
}

void MainWindow::onClipLinkUnlink() {
    statusBar()->showMessage(tr("Link/Unlink"), 2000);
}

void MainWindow::onClipVideoOptions() {
    statusBar()->showMessage(tr("Video Options"), 2000);
}

void MainWindow::onClipAudioOptions() {
    statusBar()->showMessage(tr("Audio Options"), 2000);
}

void MainWindow::onSequenceAddTrack() {
    if (m_appState != AppState::Project || !m_projectModel) return;
    m_projectModel->addTrack(true);
    statusBar()->showMessage(tr("Video track added"), 2000);
}

void MainWindow::onSequenceDeleteTrack() {
    if (m_appState != AppState::Project || !m_projectModel) return;
    int n = m_projectModel->tracks().size();
    if (n <= 1) {
        statusBar()->showMessage(tr("At least one track is required"), 3000);
        return;
    }
    m_projectModel->removeTrack(n - 1);
    statusBar()->showMessage(tr("Track removed"), 2000);
}

void MainWindow::onSequenceRenderInToOut() {
    statusBar()->showMessage(tr("Render In to Out"), 2000);
}

void MainWindow::onAddMarker() {
    statusBar()->showMessage(tr("Add Marker"), 2000);
}

void MainWindow::onNextMarker() {
    statusBar()->showMessage(tr("Go to Next Marker"), 2000);
}

void MainWindow::onPrevMarker() {
    statusBar()->showMessage(tr("Go to Previous Marker"), 2000);
}

void MainWindow::onExport() {
    statusBar()->showMessage(tr("Export — use File > Export to render sequence"), 4000);
}

} // namespace aether
