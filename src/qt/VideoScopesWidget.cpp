#include "VideoScopesWidget.h"
#include <QPainter>

namespace aether {

VideoScopesWidget::VideoScopesWidget(QWidget* parent) : QWidget(parent) {
    setMinimumSize(200, 120);
    setStyleSheet("VideoScopesWidget { background: #0d0d0d; }");
}

void VideoScopesWidget::setFrame(const QImage& frame) {
    m_frame = frame;
    update();
}

void VideoScopesWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QColor(13, 13, 13));
    p.setPen(QColor(80, 80, 80));
    p.drawText(rect(), Qt::AlignCenter, tr("Waveform | Vectorscope"));
}

} // namespace aether
