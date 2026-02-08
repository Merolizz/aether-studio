#include "PlaceholderPageWidget.h"

#include <QVBoxLayout>

namespace aether {

PlaceholderPageWidget::PlaceholderPageWidget(const QString& pageName, QWidget* parent)
    : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    QLabel* label = new QLabel(pageName, this);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("color: #666; font-size: 18px;");
    layout->addWidget(label, 1, Qt::AlignCenter);
}

} // namespace aether
