#pragma once

#include <QWidget>

class QButtonGroup;
class QHBoxLayout;

namespace aether {

class PageBarWidget : public QWidget {
    Q_OBJECT
public:
    enum class Page {
        Media = 0,
        Edit = 1,
        Animation = 2,
        Color = 3,
        Audio = 4,
        Deliver = 5
    };
    Q_ENUM(Page)

    static constexpr int PageCount = 6;

    explicit PageBarWidget(QWidget* parent = nullptr);

    int currentPage() const { return m_currentPage; }
    void setCurrentPage(int index);

signals:
    void pageChanged(int index);

private:
    void onButtonClicked(int id);

    QButtonGroup* m_buttonGroup = nullptr;
    QHBoxLayout* m_layout = nullptr;
    int m_currentPage = 1; // default EDIT
};

} // namespace aether
