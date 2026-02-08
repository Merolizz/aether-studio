#include "RenderQueueWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>

namespace aether {

RenderQueueWidget::RenderQueueWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Render Queue"), this));
    QTableWidget* table = new QTableWidget(0, 5, this);
    table->setHorizontalHeaderLabels({ tr("Name"), tr("Status"), tr("Output"), tr("Codec"), tr("Progress") });
    layout->addWidget(table, 1);
    QPushButton* addBtn = new QPushButton(tr("Add"), this);
    QPushButton* renderBtn = new QPushButton(tr("Render All"), this);
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(renderBtn);
    layout->addLayout(btnLayout);
}

} // namespace aether
