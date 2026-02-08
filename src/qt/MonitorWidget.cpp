#include "MonitorWidget.h"
#include "aether/PlaybackEngine.h"
#include <QLabel>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>
#include <QSlider>
#include <QKeySequence>
#include <QUrl>
#include <QKeyEvent>
#include <QTimer>
#include <QPixmap>
#ifdef AETHER_QT_MULTIMEDIA
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#endif

namespace aether {

void MonitorWidget::setProjectFps(int fps) {
    if (fps < 1) fps = 1;
    if (fps > 120) fps = 120;
    m_projectFps = fps;
    updateTimecodeLabel();
}

QString MonitorWidget::msToTimecode(qint64 ms) const {
    if (ms < 0) ms = 0;
    int totalSec = int(ms / 1000);
    int h = totalSec / 3600;
    int m = (totalSec % 3600) / 60;
    int s = totalSec % 60;
    int f = static_cast<int>((ms % 1000) * m_projectFps / 1000);
    if (f >= m_projectFps) f = m_projectFps - 1;
    return QString("%1:%2:%3:%4")
        .arg(h, 2, 10, QChar('0'))
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'))
        .arg(f, 2, 10, QChar('0'));
}

MonitorWidget::MonitorWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(320, 240);
    setStyleSheet("MonitorWidget { background: #0d0d0d; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    m_videoContainer = new QWidget(this);
    m_videoContainer->setStyleSheet("background: #0d0d0d;");
    QVBoxLayout* videoLayout = new QVBoxLayout(m_videoContainer);
    videoLayout->setContentsMargins(0, 0, 0, 0);

#ifdef AETHER_QT_MULTIMEDIA
    m_videoWidget = new QVideoWidget(m_videoContainer);
    m_videoWidget->setStyleSheet("background: #0d0d0d;");
    videoLayout->addWidget(m_videoWidget);
    m_engineFrameLabel = new QLabel(m_videoContainer);
    m_engineFrameLabel->setStyleSheet("background: #0d0d0d;");
    m_engineFrameLabel->setAlignment(Qt::AlignCenter);
    m_engineFrameLabel->setScaledContents(false);
    m_engineFrameLabel->hide();
    videoLayout->addWidget(m_engineFrameLabel);
#endif

#ifdef AETHER_QT_MULTIMEDIA
    m_player = new QMediaPlayer(this);
    QAudioOutput* audioOut = new QAudioOutput(this);
    m_player->setAudioOutput(audioOut);
    m_player->setVideoOutput(m_videoWidget);
    connect(m_player, &QMediaPlayer::positionChanged, this, &MonitorWidget::onPositionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &MonitorWidget::onDurationChanged);
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, &MonitorWidget::onPlaybackStateChanged);
    connect(m_player, &QMediaPlayer::errorOccurred, this, [this](QMediaPlayer::Error error, const QString& msg) {
        Q_UNUSED(error);
        emit playbackError(msg);
    });
#else
    m_engineFrameLabel = new QLabel(m_videoContainer);
    m_engineFrameLabel->setStyleSheet("background: #0d0d0d;");
    m_engineFrameLabel->setAlignment(Qt::AlignCenter);
    m_engineFrameLabel->hide();
    videoLayout->addWidget(m_engineFrameLabel);
    QLabel* noVideo = new QLabel(tr("Program Monitor"), m_videoContainer);
    noVideo->setAlignment(Qt::AlignCenter);
    noVideo->setStyleSheet("color: #666; font-size: 14px;");
    videoLayout->addWidget(noVideo);
#endif

    mainLayout->addWidget(m_videoContainer, 1);

    m_timecodeLabel = new QLabel(tr("00:00:00:00"), this);
    m_timecodeLabel->setStyleSheet("color: #ccc; font-family: monospace; font-size: 13px; padding: 2px 6px;");
    m_durationLabel = new QLabel(tr("00:00:00:00"), this);
    m_durationLabel->setStyleSheet("color: #888; font-family: monospace; font-size: 12px; padding: 2px 6px;");

    m_seekSlider = new QSlider(Qt::Horizontal, this);
    m_seekSlider->setRange(0, 0);
    m_seekSlider->setStyleSheet(
        "QSlider::groove:horizontal { height: 4px; background: #3f3f46; border-radius: 2px; }"
        "QSlider::sub-page:horizontal { background: #5a7a9e; border-radius: 2px; }"
        "QSlider::handle:horizontal { width: 12px; margin: -4px 0; background: #888; border-radius: 6px; }"
        "QSlider::handle:horizontal:hover { background: #aaa; }"
    );
    connect(m_seekSlider, &QSlider::sliderMoved, this, [this](int v) {
        if (m_playbackEngine && m_playbackEngine->hasSource()) m_playbackEngine->seek(v);
#ifdef AETHER_QT_MULTIMEDIA
        else if (m_player) m_player->setPosition(v);
#endif
    });

    m_transportBar = new QToolBar(this);
    m_transportBar->setIconSize(QSize(22, 22));
    m_transportBar->setStyleSheet(
        "QToolBar { background: #252526; border: none; padding: 4px; spacing: 2px; }"
    );

    QAction* stepBackAct = m_transportBar->addAction(tr("Step Back"));
    stepBackAct->setShortcut(QKeySequence(Qt::AltModifier | Qt::Key_Left));
    connect(stepBackAct, &QAction::triggered, this, &MonitorWidget::stepBack);

    m_playAction = m_transportBar->addAction(tr("Play"));
    m_playAction->setShortcut(QKeySequence(Qt::Key_Space));
    connect(m_playAction, &QAction::triggered, this, &MonitorWidget::play);
    m_playAction->setEnabled(false);

    QAction* pauseAct = m_transportBar->addAction(tr("Pause"));
    connect(pauseAct, &QAction::triggered, this, &MonitorWidget::pause);

    QAction* stopAct = m_transportBar->addAction(tr("Stop"));
    stopAct->setShortcut(QKeySequence(Qt::Key_Home));
    connect(stopAct, &QAction::triggered, this, &MonitorWidget::stop);

    QAction* stepFwdAct = m_transportBar->addAction(tr("Step Forward"));
    stepFwdAct->setShortcut(QKeySequence(Qt::AltModifier | Qt::Key_Right));
    connect(stepFwdAct, &QAction::triggered, this, &MonitorWidget::stepForward);

    m_transportBar->addSeparator();

    QAction* inAct = m_transportBar->addAction(tr("Go to In"));
    inAct->setShortcut(QKeySequence(Qt::Key_Q));
    connect(inAct, &QAction::triggered, this, &MonitorWidget::goToIn);

    QAction* outAct = m_transportBar->addAction(tr("Go to Out"));
    outAct->setShortcut(QKeySequence(Qt::Key_W));
    connect(outAct, &QAction::triggered, this, &MonitorWidget::goToOut);

    QAction* setInAct = m_transportBar->addAction(tr("Set In"));
    setInAct->setShortcut(QKeySequence(Qt::Key_I));
    connect(setInAct, &QAction::triggered, this, &MonitorWidget::setInPoint);

    QAction* setOutAct = m_transportBar->addAction(tr("Set Out"));
    setOutAct->setShortcut(QKeySequence(Qt::Key_O));
    connect(setOutAct, &QAction::triggered, this, &MonitorWidget::setOutPoint);

    QWidget* bottomPanel = new QWidget(this);
    bottomPanel->setStyleSheet("background: #252526; border-top: 1px solid #3f3f46;");
    QVBoxLayout* bottomLayout = new QVBoxLayout(bottomPanel);
    bottomLayout->setContentsMargins(6, 4, 6, 4);
    bottomLayout->setSpacing(2);

    QHBoxLayout* timeLayout = new QHBoxLayout();
    timeLayout->addWidget(m_timecodeLabel);
    timeLayout->addWidget(m_seekSlider, 1);
    timeLayout->addWidget(m_durationLabel);
    bottomLayout->addLayout(timeLayout);
    bottomLayout->addWidget(m_transportBar);

    mainLayout->addWidget(bottomPanel);

    m_jklScrubTimer = new QTimer(this);
    connect(m_jklScrubTimer, &QTimer::timeout, this, [this]() {
#ifdef AETHER_QT_MULTIMEDIA
        if (!m_player) return;
        if (m_jklScrubStepMs != 0) {
            qint64 pos = m_player->position() + m_jklScrubStepMs;
            if (pos < 0) pos = 0;
            if (pos > m_player->duration()) pos = m_player->duration();
            m_player->setPosition(pos);
        }
#endif
    });
}

MonitorWidget::~MonitorWidget() {
#ifdef AETHER_QT_MULTIMEDIA
    if (m_player) m_player->stop();
#endif
}

void MonitorWidget::setPlaybackEngine(PlaybackEngine* engine) {
    if (m_playbackEngine) {
        disconnect(m_playbackEngine, nullptr, this, nullptr);
        if (m_engineFrameLabel) { m_engineFrameLabel->hide(); m_engineFrameLabel->clear(); }
#ifdef AETHER_QT_MULTIMEDIA
        if (m_videoWidget) m_videoWidget->show();
#endif
    }
    m_playbackEngine = engine;
    if (m_playbackEngine) {
        connect(m_playbackEngine, &PlaybackEngine::frameReady, this, &MonitorWidget::onEngineFrameReady);
        connect(m_playbackEngine, &PlaybackEngine::positionChanged, this, &MonitorWidget::onEnginePositionChanged);
        connect(m_playbackEngine, &PlaybackEngine::durationChanged, this, &MonitorWidget::onEngineDurationChanged);
    }
}

void MonitorWidget::setSource(const QUrl& url) {
    if (m_playbackEngine && !url.isEmpty()) {
        m_playbackEngine->setSource(url.toLocalFile());
        if (m_playAction) m_playAction->setEnabled(true);
        m_seekSlider->setRange(0, 0);
        m_outPointMs = 0;
        m_inPointMs = 0;
        m_useSequenceTime = false;
#ifdef AETHER_QT_MULTIMEDIA
        if (m_videoWidget) m_videoWidget->hide();
#endif
        if (m_engineFrameLabel) m_engineFrameLabel->show();
        updateTimecodeLabel();
        return;
    }
    if (m_engineFrameLabel) { m_engineFrameLabel->hide(); m_engineFrameLabel->clear(); }
#ifdef AETHER_QT_MULTIMEDIA
    if (m_videoWidget) m_videoWidget->show();
#endif
#ifdef AETHER_QT_MULTIMEDIA
    if (m_player) {
        m_player->setSource(url);
        if (m_playAction) m_playAction->setEnabled(!url.isEmpty() && !url.toString().trimmed().isEmpty());
        m_seekSlider->setRange(0, 0);
        m_outPointMs = 0;
        m_inPointMs = 0;
        m_useSequenceTime = false;
        updateTimecodeLabel();
    }
#endif
}

void MonitorWidget::setSource(const QString& path) {
    setSource(QUrl::fromLocalFile(path));
}

void MonitorWidget::clearSource() {
    if (m_playbackEngine) m_playbackEngine->stop();
    if (m_engineFrameLabel) { m_engineFrameLabel->hide(); m_engineFrameLabel->clear(); }
#ifdef AETHER_QT_MULTIMEDIA
    if (m_videoWidget) m_videoWidget->show();
    if (m_player) m_player->setSource(QUrl());
    if (m_playAction) m_playAction->setEnabled(false);
#endif
    m_seekSlider->setRange(0, 0);
    m_sequenceTimeMs = 0;
    m_useSequenceTime = false;
    updateTimecodeLabel();
}

qint64 MonitorWidget::positionMs() const {
    if (m_playbackEngine && m_playbackEngine->hasSource()) return m_playbackEngine->getCurrentTimeMs();
#ifdef AETHER_QT_MULTIMEDIA
    return m_player ? m_player->position() : 0;
#else
    return 0;
#endif
}

qint64 MonitorWidget::durationMs() const {
    if (m_playbackEngine && m_playbackEngine->hasSource()) return m_playbackEngine->getDurationMs();
#ifdef AETHER_QT_MULTIMEDIA
    return m_player ? m_player->duration() : 0;
#else
    return 0;
#endif
}

void MonitorWidget::setPositionMs(qint64 ms) {
    if (m_playbackEngine && m_playbackEngine->hasSource()) m_playbackEngine->seek(ms);
#ifdef AETHER_QT_MULTIMEDIA
    else if (m_player) m_player->setPosition(ms);
#endif
    m_sequenceTimeMs = ms;
    m_useSequenceTime = true;
    updateTimecodeLabel();
}

void MonitorWidget::setTimecodeFromSequence(qint64 ms) {
    m_sequenceTimeMs = ms;
    m_useSequenceTime = true;
    updateTimecodeLabel();
}

void MonitorWidget::play() {
    if (m_playbackEngine && m_playbackEngine->hasSource()) { m_playbackEngine->play(); return; }
#ifdef AETHER_QT_MULTIMEDIA
    if (!m_player) return;
    if (m_player->source().isEmpty() || m_player->source().toString().trimmed().isEmpty())
        return;
    m_player->play();
#endif
}

void MonitorWidget::pause() {
    if (m_playbackEngine && m_playbackEngine->hasSource()) { m_playbackEngine->pause(); return; }
#ifdef AETHER_QT_MULTIMEDIA
    if (m_player) m_player->pause();
#endif
}

void MonitorWidget::stop() {
    if (m_playbackEngine && m_playbackEngine->hasSource()) { m_playbackEngine->stop(); return; }
#ifdef AETHER_QT_MULTIMEDIA
    if (m_player) m_player->stop();
#endif
}

void MonitorWidget::stepForward() {
#ifdef AETHER_QT_MULTIMEDIA
    if (m_player) m_player->setPosition(m_player->position() + 33333);
#endif
}

void MonitorWidget::stepBack() {
#ifdef AETHER_QT_MULTIMEDIA
    if (m_player) m_player->setPosition(qMax(0ll, m_player->position() - 33333));
#endif
}

void MonitorWidget::goToIn() {
    setPositionMs(m_inPointMs);
#ifdef AETHER_QT_MULTIMEDIA
    if (m_player) m_player->setPosition(m_inPointMs);
#endif
}

void MonitorWidget::goToOut() {
    setPositionMs(m_outPointMs);
#ifdef AETHER_QT_MULTIMEDIA
    if (m_player) m_player->setPosition(m_outPointMs);
#endif
}

void MonitorWidget::setInPoint() {
    qint64 pos = m_useSequenceTime ? m_sequenceTimeMs : positionMs();
    m_inPointMs = pos;
    emit inPointSet(pos);
    updateTimecodeLabel();
}

void MonitorWidget::setOutPoint() {
    qint64 pos = m_useSequenceTime ? m_sequenceTimeMs : positionMs();
    m_outPointMs = pos;
    emit outPointSet(pos);
    updateTimecodeLabel();
}

void MonitorWidget::onPositionChanged(qint64 pos) {
    if (!m_useSequenceTime) m_sequenceTimeMs = pos;
    m_seekSlider->blockSignals(true);
    m_seekSlider->setValue(static_cast<int>(pos));
#ifdef AETHER_QT_MULTIMEDIA
    if (m_player) m_seekSlider->setRange(0, static_cast<int>(m_player->duration()));
#endif
    m_seekSlider->blockSignals(false);
    updateTimecodeLabel();
    emit positionChanged(pos);
}

void MonitorWidget::onDurationChanged(qint64 dur) {
    m_seekSlider->setRange(0, static_cast<int>(dur));
    if (m_outPointMs <= 0) m_outPointMs = dur;
    m_durationLabel->setText(msToTimecode(dur));
    emit durationChanged(dur);
}

void MonitorWidget::onMediaStatusChanged() {}

void MonitorWidget::onPlaybackStateChanged() {}

void MonitorWidget::onEngineFrameReady() {
    if (!m_playbackEngine || !m_engineFrameLabel) return;
    QImage img = m_playbackEngine->getCurrentFrame();
    if (img.isNull()) return;
    QPixmap pm = QPixmap::fromImage(img);
    m_engineFrameLabel->setPixmap(pm.scaled(m_engineFrameLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MonitorWidget::onEnginePositionChanged(qint64 ms) {
    if (!m_useSequenceTime) m_sequenceTimeMs = ms;
    m_seekSlider->blockSignals(true);
    m_seekSlider->setValue(static_cast<int>(ms));
    if (m_playbackEngine) m_seekSlider->setRange(0, static_cast<int>(m_playbackEngine->getDurationMs()));
    m_seekSlider->blockSignals(false);
    updateTimecodeLabel();
    emit positionChanged(ms);
}

void MonitorWidget::onEngineDurationChanged(qint64 dur) {
    m_seekSlider->setRange(0, static_cast<int>(dur));
    if (m_outPointMs <= 0) m_outPointMs = dur;
    m_durationLabel->setText(msToTimecode(dur));
    emit durationChanged(dur);
}

void MonitorWidget::stopJKLScrub() {
    if (m_jklScrubTimer) m_jklScrubTimer->stop();
    m_jklScrubStepMs = 0;
#ifdef AETHER_QT_MULTIMEDIA
    if (m_player) m_player->setPlaybackRate(1.0);
#endif
}

void MonitorWidget::onJKL(int key) {
#ifdef AETHER_QT_MULTIMEDIA
    if (!m_player) return;
    stopJKLScrub();
    if (key == Qt::Key_J) {
        m_jklForwardSpeed = 1;
        m_jklScrubStepMs = -80;
        m_jklScrubTimer->start(33);
    } else if (key == Qt::Key_K) {
        m_jklForwardSpeed = 1;
        pause();
    } else if (key == Qt::Key_L) {
        m_jklForwardSpeed = (m_jklForwardSpeed >= 2) ? 1 : m_jklForwardSpeed + 1;
        m_player->setPlaybackRate(static_cast<qreal>(m_jklForwardSpeed));
        play();
    }
#endif
}

void MonitorWidget::keyPressEvent(QKeyEvent* event) {
    int k = event->key();
    if (k == Qt::Key_J || k == Qt::Key_K || k == Qt::Key_L) {
        onJKL(k);
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void MonitorWidget::updateTimecodeLabel() {
    qint64 displayMs = m_useSequenceTime ? m_sequenceTimeMs : positionMs();
    m_timecodeLabel->setText(msToTimecode(displayMs));
}

} // namespace aether
