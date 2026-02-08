#include "KeyframeTimelineWidget.h"
#include "aether/KeyframeModel.h"

#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QVBoxLayout>
#include <QLabel>

namespace aether {

KeyframeTimelineWidget::KeyframeTimelineWidget(QWidget* parent)
    : QWidget(parent) {
    setMinimumHeight(120);
    setStyleSheet("KeyframeTimelineWidget { background: #1e1e1e; }");
}

void KeyframeTimelineWidget::setModel(KeyframeModel* model) {
    m_model = model;
    update();
}

void KeyframeTimelineWidget::setCurrentTimeMs(int64_t timeMs) {
    if (m_currentTimeMs != timeMs) {
        m_currentTimeMs = timeMs;
        update();
        emit currentTimeChanged(timeMs);
    }
}

void KeyframeTimelineWidget::updateLayout() {
    update();
}

int KeyframeTimelineWidget::timeToPixel(int64_t ms) const {
    return m_paramNameWidth + static_cast<int>(ms * m_pixelsPerMs / 1000);
}

int64_t KeyframeTimelineWidget::pixelToTime(int x) const {
    if (x < m_paramNameWidth) return 0;
    return static_cast<int64_t>((x - m_paramNameWidth) * 1000) / m_pixelsPerMs;
}

QRect KeyframeTimelineWidget::keyframeRect(int paramIndex, int keyframeIndex) const {
    if (!m_model || paramIndex < 0 || paramIndex >= static_cast<int>(m_model->parameters().size())) return QRect();
    const auto& params = m_model->parameters();
    if (keyframeIndex < 0 || keyframeIndex >= static_cast<int>(params[paramIndex].keyframes.size())) return QRect();
    int64_t t = params[paramIndex].keyframes[keyframeIndex].timeMs;
    int x = timeToPixel(t);
    int y = m_rulerHeight + paramIndex * m_rowHeight + m_rowHeight / 2 - 6;
    return QRect(x - 6, y, 12, 12);
}

void KeyframeTimelineWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.fillRect(rect(), QColor(30, 30, 34));

    int w = width();
    int h = height();
    p.setPen(QPen(QColor(60, 60, 66), 1));
    p.drawLine(m_paramNameWidth, 0, m_paramNameWidth, h);
    p.drawLine(0, m_rulerHeight, w, m_rulerHeight);

    for (int x = m_paramNameWidth; x < w; x += 50)
        p.drawLine(x, 0, x, m_rulerHeight);
    p.setPen(QColor(180, 180, 180));
    p.drawText(QRect(4, 0, m_paramNameWidth - 4, m_rulerHeight), Qt::AlignVCenter | Qt::AlignLeft, tr("Time"));

    int playheadX = timeToPixel(m_currentTimeMs);
    p.setPen(QPen(QColor(255, 100, 100), 2));
    p.drawLine(playheadX, 0, playheadX, h);

    if (!m_model) return;
    const auto& params = m_model->parameters();
    for (size_t i = 0; i < params.size(); i++) {
        int rowY = m_rulerHeight + static_cast<int>(i) * m_rowHeight;
        p.setPen(QColor(200, 200, 200));
        p.drawText(QRect(4, rowY, m_paramNameWidth - 4, m_rowHeight), Qt::AlignVCenter | Qt::AlignLeft,
            QString::fromStdString(params[i].displayName));
        p.setPen(QPen(QColor(70, 70, 74), 1));
        p.drawLine(m_paramNameWidth, rowY + m_rowHeight - 1, w, rowY + m_rowHeight - 1);
        for (size_t k = 0; k < params[i].keyframes.size(); k++) {
            QRect kr = keyframeRect(static_cast<int>(i), static_cast<int>(k));
            p.setBrush(QColor(94, 129, 172));
            p.setPen(QPen(QColor(70, 100, 140), 1));
            QPolygonF diamond;
            diamond << QPointF(kr.center().x(), kr.top()) << QPointF(kr.right(), kr.center().y())
                    << QPointF(kr.center().x(), kr.bottom()) << QPointF(kr.left(), kr.center().y());
            p.drawPolygon(diamond);
        }
    }
}

void KeyframeTimelineWidget::mousePressEvent(QMouseEvent* event) {
    if (event->pos().y() < m_rulerHeight && event->button() == Qt::LeftButton) {
        m_currentTimeMs = pixelToTime(event->pos().x());
        update();
        emit currentTimeChanged(m_currentTimeMs);
    }
    QWidget::mousePressEvent(event);
}

void KeyframeTimelineWidget::mouseMoveEvent(QMouseEvent* event) {
    QWidget::mouseMoveEvent(event);
}

void KeyframeTimelineWidget::wheelEvent(QWheelEvent* event) {
    if (event->angleDelta().y() != 0) {
        int delta = event->angleDelta().y() > 0 ? 1 : -1;
        m_pixelsPerMs = qBound(1, m_pixelsPerMs + delta, 50);
        update();
    }
    event->accept();
}

} // namespace aether
