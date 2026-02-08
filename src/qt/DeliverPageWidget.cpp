#include "DeliverPageWidget.h"
#include "RenderQueueWidget.h"
#include <QVBoxLayout>

namespace aether {

DeliverPageWidget::DeliverPageWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new RenderQueueWidget(this), 1);
}

} // namespace aether
