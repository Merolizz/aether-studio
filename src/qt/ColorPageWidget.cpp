#include "ColorPageWidget.h"
#include "ColorWheelsWidget.h"
#include "VideoScopesWidget.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QLabel>

namespace aether {

ColorPageWidget::ColorPageWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    QSplitter* split = new QSplitter(Qt::Horizontal, this);
    m_wheels = new ColorWheelsWidget(this);
    m_scopes = new VideoScopesWidget(this);
    split->addWidget(m_wheels);
    split->addWidget(m_scopes);
    split->setStretchFactor(0, 1);
    split->setStretchFactor(1, 1);
    mainLayout->addWidget(split);
}

} // namespace aether
