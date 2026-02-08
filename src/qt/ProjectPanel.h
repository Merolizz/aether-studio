#pragma once

#include <QDockWidget>
#include <QPointer>

class QListWidget;
class QToolBar;
class QListWidgetItem;

namespace aether {

class ProjectModel;

class ProjectPanel : public QDockWidget {
    Q_OBJECT
public:
    explicit ProjectPanel(ProjectModel* model, QWidget* parent = nullptr);

    void setModel(ProjectModel* model);
    QString currentMediaPath() const;

signals:
    void importRequested();
    void filesDropped(const QStringList& paths);
    void addToTimelineRequested(const QString& mediaPath);
    void newBinRequested();
    void createProxyRequested(const QString& mediaPath);

private slots:
    void onImport();
    void onFilesDropped(const QStringList& paths);
    void onItemDoubleClicked(QListWidgetItem* item);
    void onMediaListChanged();
    void onNewBin();
    void onContextMenu(const QPoint& pos);
    void onInterpretFootage(const QString& path);

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    void refreshList();

    QPointer<ProjectModel> m_model;
    QListWidget* m_list = nullptr;
    QToolBar* m_toolBar = nullptr;
};

} // namespace aether
