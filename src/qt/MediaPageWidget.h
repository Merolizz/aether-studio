#pragma once

#include <QWidget>
#include <QPointer>

class QListWidget;
class QListWidgetItem;
class QToolBar;
class QVBoxLayout;

namespace aether {

class ProjectModel;

class MediaPageWidget : public QWidget {
    Q_OBJECT
public:
    explicit MediaPageWidget(ProjectModel* model, QWidget* parent = nullptr);

    void setModel(ProjectModel* model);

signals:
    void importRequested();
    void addToTimelineRequested(const QString& mediaPath);

private slots:
    void onImport();
    void onItemDoubleClicked(QListWidgetItem* item);
    void onMediaListChanged();

private:
    void refreshList();

    QPointer<ProjectModel> m_model;
    QListWidget* m_list = nullptr;
    QToolBar* m_toolBar = nullptr;
};

} // namespace aether
