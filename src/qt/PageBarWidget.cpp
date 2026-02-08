#include "PageBarWidget.h"

#include <QPushButton>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QFrame>
#include <QSpacerItem>

namespace aether {

PageBarWidget::PageBarWidget(QWidget* parent)
    : QWidget(parent) {
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(8, 6, 8, 6);
    m_layout->setSpacing(4);

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);

    const char* labels[] = { "MEDIA", "EDIT", "ANIMATION", "COLOR", "AUDIO", "DELIVER" };
    for (int i = 0; i < PageCount; i++) {
        QPushButton* btn = new QPushButton(tr(labels[i]), this);
        btn->setCheckable(true);
        btn->setFixedHeight(32);
        btn->setMinimumWidth(80);
        btn->setProperty("pageIndex", i);
        m_buttonGroup->addButton(btn, i);
        m_layout->addWidget(btn);
        btn->setChecked(i == static_cast<int>(Page::Edit));
    }
    m_currentPage = static_cast<int>(Page::Edit);

    setStyleSheet(
        "PageBarWidget { background: #252526; border-top: 1px solid #3c3c3c; }"
        "QPushButton { background: #2d2d32; color: #cccccc; border: 1px solid #3c3c3c; border-radius: 4px; font-weight: 500; }"
        "QPushButton:hover { background: #3c3c3c; color: #ffffff; }"
        "QPushButton:checked { background: #0e639c; color: #ffffff; border-color: #1177bb; }"
    );
    setFixedHeight(44);

    connect(m_buttonGroup, QOverload<int>::of(&QButtonGroup::idClicked), this, &PageBarWidget::onButtonClicked);
}

void PageBarWidget::setCurrentPage(int index) {
    if (index < 0 || index >= PageCount || index == m_currentPage) return;
    m_currentPage = index;
    QAbstractButton* b = m_buttonGroup->button(index);
    if (b) b->setChecked(true);
}

void PageBarWidget::onButtonClicked(int id) {
    if (id < 0 || id >= PageCount) return;
    m_currentPage = id;
    emit pageChanged(id);
}

} // namespace aether
