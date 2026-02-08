#include "ProjectPanel.h"
#include "ProjectModel.h"
#include <QListWidget>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWidget>
#include <QAction>
#include <QFileInfo>
#include <QMessageBox>
#include <QMenu>
#include <QDialog>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QEvent>

namespace aether {

ProjectPanel::ProjectPanel(ProjectModel* model, QWidget* parent)
    : QDockWidget(parent), m_model(model) {
    setWindowTitle(tr("Project"));
    setObjectName("ProjectPanel");

    QWidget* content = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);

    m_toolBar = new QToolBar(this);
    m_toolBar->setIconSize(QSize(18, 18));
    m_toolBar->setStyleSheet("QToolBar { background: transparent; border: none; }");

    QAction* newBinAct = m_toolBar->addAction(tr("New Bin"));
    newBinAct->setStatusTip(tr("Create new bin"));
    connect(newBinAct, &QAction::triggered, this, &ProjectPanel::onNewBin);

    m_toolBar->addSeparator();

    QAction* importAct = m_toolBar->addAction(tr("Import"));
    importAct->setStatusTip(tr("Import media files"));
    connect(importAct, &QAction::triggered, this, &ProjectPanel::onImport);

    layout->addWidget(m_toolBar);

    m_list = new QListWidget(this);
    m_list->setAcceptDrops(true);
    m_list->setContextMenuPolicy(Qt::CustomContextMenu);
    m_list->setDragDropMode(QListWidget::DragOnly);
    m_list->installEventFilter(this);
    m_list->setStyleSheet(
        "QListWidget { background: #1e1e1e; border: none; outline: none; }"
        "QListWidget::item { padding: 4px 8px; }"
        "QListWidget::item:selected { background: #2d5a87; }"
        "QListWidget::item:hover { background: #2d2d32; }"
    );
    connect(m_list, &QListWidget::itemDoubleClicked, this, &ProjectPanel::onItemDoubleClicked);
    connect(m_list, &QListWidget::customContextMenuRequested, this, &ProjectPanel::onContextMenu);
    layout->addWidget(m_list, 1);

    setWidget(content);

    if (m_model) {
        connect(m_model, &ProjectModel::mediaListChanged, this, &ProjectPanel::onMediaListChanged);
        refreshList();
    }
}

void ProjectPanel::setModel(ProjectModel* model) {
    if (m_model)
        disconnect(m_model, nullptr, this, nullptr);
    m_model = model;
    if (m_model) {
        connect(m_model, &ProjectModel::mediaListChanged, this, &ProjectPanel::onMediaListChanged);
        refreshList();
    }
}

QString ProjectPanel::currentMediaPath() const {
    QListWidgetItem* item = m_list->currentItem();
    if (!item || !item->data(Qt::UserRole).isValid()) return QString();
    return item->data(Qt::UserRole).toString();
}

void ProjectPanel::onImport() {
    emit importRequested();
}

void ProjectPanel::onFilesDropped(const QStringList& paths) {
    emit filesDropped(paths);
}

bool ProjectPanel::eventFilter(QObject* obj, QEvent* event) {
    if (obj != m_list) return QDockWidget::eventFilter(obj, event);
    if (event->type() == QEvent::DragEnter) {
        auto* e = static_cast<QDragEnterEvent*>(event);
        if (e->mimeData() && e->mimeData()->hasUrls()) e->acceptProposedAction();
        return true;
    }
    if (event->type() == QEvent::Drop) {
        auto* e = static_cast<QDropEvent*>(event);
        if (e->mimeData()) {
            QStringList paths;
            for (const QUrl& url : e->mimeData()->urls()) {
                QString path = url.toLocalFile();
                if (!path.isEmpty()) paths.append(path);
            }
            if (!paths.isEmpty()) emit filesDropped(paths);
        }
        e->acceptProposedAction();
        return true;
    }
    return QDockWidget::eventFilter(obj, event);
}

void ProjectPanel::onItemDoubleClicked(QListWidgetItem* item) {
    if (!item || !item->data(Qt::UserRole).isValid()) return;
    emit addToTimelineRequested(item->data(Qt::UserRole).toString());
}

void ProjectPanel::onMediaListChanged() {
    refreshList();
}

void ProjectPanel::onNewBin() {
    emit newBinRequested();
}

void ProjectPanel::onContextMenu(const QPoint& pos) {
    QListWidgetItem* item = m_list->itemAt(pos);
    QMenu menu(this);
    QAction* importAct = menu.addAction(tr("Import..."));
    connect(importAct, &QAction::triggered, this, &ProjectPanel::onImport);
    if (item && item->data(Qt::UserRole).isValid()) {
        QString path = item->data(Qt::UserRole).toString();
        if (!path.isEmpty()) {
            QAction* proxyAct = menu.addAction(tr("Create proxy"));
            connect(proxyAct, &QAction::triggered, this, [this, path]() { emit createProxyRequested(path); });
            QAction* interpretAct = menu.addAction(tr("Interpret Footage…"));
            connect(interpretAct, &QAction::triggered, this, [this, path]() { onInterpretFootage(path); });
        }
    }
    menu.exec(m_list->mapToGlobal(pos));
}

void ProjectPanel::onInterpretFootage(const QString& path) {
    if (!m_model) return;
    QDialog dlg(this);
    dlg.setWindowTitle(tr("Interpret Footage"));
    QFormLayout* form = new QFormLayout(&dlg);
    QSpinBox* fpsSpin = new QSpinBox(&dlg);
    fpsSpin->setRange(0, 120);
    fpsSpin->setSpecialValueText(tr("Use file default"));
    fpsSpin->setValue(m_model->mediaInterpretFps(path));
    form->addRow(tr("Assume frame rate (fps):"), fpsSpin);
    QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    form->addRow(box);
    connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
    if (dlg.exec() == QDialog::Accepted)
        m_model->setMediaInterpretFpsByPath(path, fpsSpin->value());
}

void ProjectPanel::refreshList() {
    m_list->clear();
    if (!m_model) return;
    for (const MediaItem& m : m_model->media()) {
        QListWidgetItem* item = new QListWidgetItem(m.name, m_list);
        item->setData(Qt::UserRole, m.path);
        item->setToolTip(m.path);
        if (m.durationMs > 0)
            item->setData(Qt::UserRole + 1, m.durationMs);
    }
}

} // namespace aether
