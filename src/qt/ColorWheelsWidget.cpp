#include "ColorWheelsWidget.h"
#include "aether/ColorGradeState.h"
#include <QPainter>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QLabel>

namespace aether {

ColorWheelsWidget::ColorWheelsWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(280, 200);
    setStyleSheet("ColorWheelsWidget { background: #1e1e1e; }");
    QVBoxLayout* layout = new QVBoxLayout(this);
    QLabel* title = new QLabel(tr("Lift  |  Gamma  |  Gain"), this);
    title->setStyleSheet("color: #888;");
    layout->addWidget(title);
}

void ColorWheelsWidget::getState(ColorGradeState* out) const {
    if (out) *out = m_state;
}

void ColorWheelsWidget::setState(const ColorGradeState& state) {
    m_state = state;
    update();
}

void ColorWheelsWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QColor(30, 30, 34));
    p.setPen(QColor(100, 100, 100));
    p.drawText(rect(), Qt::AlignCenter, tr("3-Way Color Wheels"));
}

void ColorWheelsWidget::mousePressEvent(QMouseEvent* e) { Q_UNUSED(e); }
void ColorWheelsWidget::mouseMoveEvent(QMouseEvent* e) { Q_UNUSED(e); }

} // namespace aether
