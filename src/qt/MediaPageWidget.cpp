#include "MediaPageWidget.h"
#include "ProjectModel.h"

#include <QListWidget>
#include <QToolBar>
#include <QVBoxLayout>
#include <QAction>
#include <QLabel>
#include <QFileInfo>

namespace aether {

MediaPageWidget::MediaPageWidget(ProjectModel* model, QWidget* parent)
    : QWidget(parent), m_model(model) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(12, 12, 12, 12);

    m_toolBar = new QToolBar(this);
    m_toolBar->setIconSize(QSize(18, 18));
    m_toolBar->setStyleSheet("QToolBar { background: transparent; border: none; }");
    QAction* importAct = m_toolBar->addAction(tr("Import Media..."));
    connect(importAct, &QAction::triggered, this, &MediaPageWidget::onImport);
    layout->addWidget(m_toolBar);

    QLabel* title = new QLabel(tr("Media — Project assets"), this);
    title->setStyleSheet("color: #888; font-size: 12px; padding: 4px 0;");
    layout->addWidget(title);

    m_list = new QListWidget(this);
    m_list->setDragDropMode(QListWidget::DragOnly);
    m_list->setStyleSheet(
        "QListWidget { background: #1e1e1e; border: 1px solid #3c3c3c; border-radius: 4px; }"
        "QListWidget::item { padding: 6px 10px; }"
        "QListWidget::item:selected { background: #2d5a87; }"
        "QListWidget::item:hover { background: #2d2d32; }"
    );
    connect(m_list, &QListWidget::itemDoubleClicked, this, &MediaPageWidget::onItemDoubleClicked);
    layout->addWidget(m_list, 1);

    if (m_model) {
        connect(m_model, &ProjectModel::mediaListChanged, this, &MediaPageWidget::onMediaListChanged);
        refreshList();
    }
}

void MediaPageWidget::setModel(ProjectModel* model) {
    if (m_model)
        disconnect(m_model, nullptr, this, nullptr);
    m_model = model;
    if (m_model) {
        connect(m_model, &ProjectModel::mediaListChanged, this, &MediaPageWidget::onMediaListChanged);
        refreshList();
    }
}

void MediaPageWidget::onImport() {
    emit importRequested();
}

void MediaPageWidget::onItemDoubleClicked(QListWidgetItem* item) {
    if (!item || !item->data(Qt::UserRole).isValid()) return;
    emit addToTimelineRequested(item->data(Qt::UserRole).toString());
}

void MediaPageWidget::onMediaListChanged() {
    refreshList();
}

void MediaPageWidget::refreshList() {
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
