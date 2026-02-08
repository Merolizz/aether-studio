#pragma once

#include <QWidget>

class QPushButton;
class QListWidget;
class QShowEvent;

namespace aether {

class HomeWidget : public QWidget {
    Q_OBJECT
public:
    explicit HomeWidget(QWidget* parent = nullptr);

signals:
    void newProjectRequested();
    void openProjectRequested();
    void openRecentRequested(const QString& path);

protected:
    void showEvent(QShowEvent* event) override;

private:
    void onNewProject();
    void onOpenProject();
    void onRecentClicked();
    void refreshRecentList();

    QPushButton* m_newProjectBtn = nullptr;
    QPushButton* m_openProjectBtn = nullptr;
    QListWidget* m_recentList = nullptr;
};

} // namespace aether
