#include "TimelineWidget.h"
#include "ProjectModel.h"
#include <QPainter>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QFileInfo>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollArea>
#include <QApplication>
#include <QMenu>
#include <QDialog>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSlider>

namespace aether {

static constexpr double kPpmMin = 0.02;
static constexpr double kPpmMax = 2.0;
static int ppmToSliderValue(double ppm) {
    double v = (ppm - kPpmMin) / (kPpmMax - kPpmMin) * 100.0;
    if (v < 0) return 0;
    if (v > 100) return 100;
    return static_cast<int>(v + 0.5);
}
static double sliderValueToPpm(int value) {
    return kPpmMin + (value / 100.0) * (kPpmMax - kPpmMin);
}

static QString msToTimecode(qint64 ms) {
    if (ms < 0) ms = 0;
    int totalSec = int(ms / 1000);
    int h = totalSec / 3600;
    int m = (totalSec % 3600) / 60;
    int s = totalSec % 60;
    int fr = int((ms % 1000) * 30 / 1000);
    return QString("%1:%2:%3:%4")
        .arg(h, 2, 10, QChar('0'))
        .arg(m, 2, 10, QChar('0'))
        .arg(s, 2, 10, QChar('0'))
        .arg(qMin(29, fr), 2, 10, QChar('0'));
}

TimelineWidget::TimelineWidget(ProjectModel* model, QWidget* parent)
    : QWidget(parent), m_model(model) {
    setMinimumHeight(120);
    setStyleSheet("TimelineWidget { background: #1c1c1f; }");
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);

    m_hScroll = new QScrollBar(Qt::Horizontal, this);
    m_vScroll = new QScrollBar(Qt::Vertical, this);
    m_hScroll->setStyleSheet("QScrollBar:horizontal { height: 14px; background: #252526; border: none; }");
    m_vScroll->setStyleSheet("QScrollBar:vertical { width: 14px; background: #252526; border: none; }");

    connect(m_hScroll, &QScrollBar::valueChanged, this, [this]() { update(); });
    connect(m_vScroll, &QScrollBar::valueChanged, this, [this]() { update(); });

    m_zoomSlider = new QSlider(Qt::Horizontal, this);
    m_zoomSlider->setRange(0, 100);
    m_zoomSlider->setValue(ppmToSliderValue(m_pixelsPerMs));
    m_zoomSlider->setStyleSheet("QSlider::groove:horizontal { height: 6px; background: #252526; } QSlider::handle:horizontal { width: 10px; background: #5a7a9e; border-radius: 5px; }");
    connect(m_zoomSlider, &QSlider::valueChanged, this, [this](int v) {
        setPixelsPerMs(sliderValueToPpm(v));
    });

    if (m_model) {
        connect(m_model, &ProjectModel::tracksChanged, this, [this]() { updateScrollRange(); update(); });
    }
    m_pixelsPerMs = 0.15;
    updateScrollRange();
}

void TimelineWidget::setModel(ProjectModel* model) {
    if (m_model) disconnect(m_model, nullptr, this, nullptr);
    m_model = model;
    if (m_model) {
        connect(m_model, &ProjectModel::tracksChanged, this, [this]() { updateScrollRange(); update(); });
    }
    updateScrollRange();
    update();
}

void TimelineWidget::setPlayheadPositionMs(qint64 ms) {
    if (m_playheadMs == ms) return;
    m_playheadMs = qMax(0ll, ms);
    emit playheadMoved(m_playheadMs);
    update();
}

void TimelineWidget::setPixelsPerMs(double ppm) {
    if (ppm < kPpmMin) ppm = kPpmMin;
    if (ppm > kPpmMax) ppm = kPpmMax;
    m_pixelsPerMs = ppm;
    if (m_zoomSlider) {
        m_zoomSlider->blockSignals(true);
        m_zoomSlider->setValue(ppmToSliderValue(m_pixelsPerMs));
        m_zoomSlider->blockSignals(false);
    }
    updateScrollRange();
    update();
}

void TimelineWidget::zoomIn() {
    setPixelsPerMs(m_pixelsPerMs * 1.2);
}

void TimelineWidget::zoomOut() {
    setPixelsPerMs(m_pixelsPerMs / 1.2);
}

void TimelineWidget::zoomFit() {
    if (!m_model || m_model->sequenceDurationMs() <= 0) return;
    int w = width() - m_trackHeaderWidth - (m_vScroll->isVisible() ? m_vScroll->width() : 0);
    if (w <= 0) return;
    m_pixelsPerMs = static_cast<double>(w) / m_model->sequenceDurationMs();
    if (m_pixelsPerMs > 2.0) m_pixelsPerMs = 2.0;
    if (m_pixelsPerMs < 0.02) m_pixelsPerMs = 0.02;
    updateScrollRange();
    update();
}

void TimelineWidget::setTrackHeight(int h) {
    if (h < 24) h = 24;
    if (h > 120) h = 120;
    m_trackHeight = h;
    updateScrollRange();
    update();
}

void TimelineWidget::setTool(Tool t) {
    m_tool = t;
}

int TimelineWidget::timeToPixel(qint64 ms) const {
    return static_cast<int>(ms * m_pixelsPerMs);
}

qint64 TimelineWidget::pixelToTime(int x) const {
    return static_cast<qint64>(x / m_pixelsPerMs);
}

void TimelineWidget::updateScrollRange() {
    qint64 dur = m_model ? m_model->sequenceDurationMs() : 60000;
    if (dur < 60000) dur = 60000;
    m_contentWidth = timeToPixel(dur) + 200;
    int trackCount = m_model ? m_model->tracks().size() : 2;
    m_contentHeight = m_rulerHeight + trackCount * m_trackHeight;

    int viewW = width() - m_trackHeaderWidth - (m_vScroll->isVisible() ? m_vScroll->width() : 0);
    int viewH = height() - m_rulerHeight - (m_hScroll->isVisible() ? m_hScroll->height() : 0);

    m_hScroll->setRange(0, qMax(0, m_contentWidth - viewW));
    m_hScroll->setPageStep(viewW);
    m_vScroll->setRange(0, qMax(0, m_contentHeight - viewH));
    m_vScroll->setPageStep(viewH);
}

QRect TimelineWidget::trackHeaderRect(int trackIndex) const {
    int y = m_rulerHeight + trackIndex * m_trackHeight;
    return QRect(0, y, m_trackHeaderWidth, m_trackHeight);
}

QRect TimelineWidget::trackContentRect(int trackIndex) const {
    int y = m_rulerHeight + trackIndex * m_trackHeight;
    int w = width() - m_trackHeaderWidth - (m_vScroll->isVisible() ? m_vScroll->width() : 0);
    return QRect(m_trackHeaderWidth, y, qMax(0, w), m_trackHeight);
}

QRect TimelineWidget::clipRect(int trackIndex, int clipIndex) const {
    if (!m_model || trackIndex < 0 || trackIndex >= m_model->tracks().size()) return QRect();
    const Track& track = m_model->tracks()[trackIndex];
    if (clipIndex < 0 || clipIndex >= track.clips.size()) return QRect();
    const TimelineClip& c = track.clips[clipIndex];
    qint64 durationOnTimelineMs = (c.speedRatio > 0.001)
        ? static_cast<qint64>((c.sourceOutMs - c.sourceInMs) / c.speedRatio)
        : (c.sourceOutMs - c.sourceInMs);
    int x = timeToPixel(c.timelineStartMs) - m_hScroll->value();
    int w = timeToPixel(durationOnTimelineMs);
    int y = m_rulerHeight + trackIndex * m_trackHeight + 2;
    int h = m_trackHeight - 4;
    return QRect(m_trackHeaderWidth + x, y, qMax(4, w), h);
}

int TimelineWidget::trackAtY(int y) const {
    if (y < m_rulerHeight) return -1;
    int t = (y - m_rulerHeight) / m_trackHeight;
    int count = m_model ? m_model->tracks().size() : 0;
    if (t >= count) return -1;
    return t;
}

void TimelineWidget::findClipAt(int x, int y, int* outTrack, int* outClip) const {
    *outTrack = -1;
    *outClip = -1;
    if (!m_model) return;
    int t = trackAtY(y);
    if (t < 0) return;
    int contentX = x - m_trackHeaderWidth + m_hScroll->value();
    qint64 timeMs = pixelToTime(contentX);
    const Track& track = m_model->tracks()[t];
    for (int i = 0; i < track.clips.size(); ++i) {
        const TimelineClip& c = track.clips[i];
        qint64 start = c.timelineStartMs;
        qint64 durationOnTimelineMs = (c.speedRatio > 0.001)
            ? static_cast<qint64>((c.sourceOutMs - c.sourceInMs) / c.speedRatio)
            : (c.sourceOutMs - c.sourceInMs);
        qint64 end = c.timelineStartMs + durationOnTimelineMs;
        if (timeMs >= start && timeMs < end) {
            *outTrack = t;
            *outClip = i;
            return;
        }
    }
}

void TimelineWidget::paintEvent(QPaintEvent* e) {
    Q_UNUSED(e);
    QPainter p(this);
    p.fillRect(rect(), QColor(28, 28, 31));

    int viewW = width();
    int viewH = height();
    int scrollX = m_hScroll->value();
    int scrollY = m_vScroll->value();

    // Ruler
    p.fillRect(0, 0, viewW, m_rulerHeight, QColor(37, 37, 38));
    p.setPen(QColor(100, 100, 100));
    p.drawLine(0, m_rulerHeight - 1, viewW, m_rulerHeight - 1);

    qint64 visibleStart = pixelToTime(scrollX);
    qint64 visibleEnd = pixelToTime(scrollX + viewW - m_trackHeaderWidth);
    qint64 step = 1000;
    if (m_pixelsPerMs > 0.5) step = 100;
    else if (m_pixelsPerMs > 0.2) step = 500;
    qint64 t = (visibleStart / step) * step;
    while (t <= visibleEnd + step) {
        int x = m_trackHeaderWidth + timeToPixel(t) - scrollX;
        if (x >= m_trackHeaderWidth) {
            bool major = (t % 5000 == 0);
            p.setPen(major ? QColor(180, 180, 180) : QColor(80, 80, 80));
            p.drawLine(x, major ? 4 : 10, x, m_rulerHeight - 1);
            if (major)
                p.drawText(QRect(x - 30, 0, 60, 22), Qt::AlignCenter, msToTimecode(t));
        }
        t += step;
    }

    // Track headers and content
    int trackCount = m_model ? m_model->tracks().size() : 2;
    for (int i = 0; i < trackCount; ++i) {
        QRect headerR = trackHeaderRect(i);
        headerR.translate(0, -scrollY);
        if (headerR.bottom() < 0 || headerR.top() > viewH) continue;
        p.fillRect(headerR, QColor(45, 45, 50));
        p.setPen(QColor(80, 80, 80));
        p.drawLine(headerR.right(), headerR.top(), headerR.right(), headerR.bottom());
        QString name = m_model ? m_model->tracks()[i].name : QString("Track %1").arg(i + 1);
        p.setPen(QColor(200, 200, 200));
        p.drawText(headerR.adjusted(4, 0, -4, 0), Qt::AlignVCenter | Qt::AlignLeft, name);

        QRect contentR = trackContentRect(i);
        contentR.translate(0, -scrollY);
        if (contentR.bottom() < 0 || contentR.top() > viewH) continue;
        p.fillRect(contentR, (i % 2 == 0) ? QColor(30, 30, 34) : QColor(26, 26, 29));

        if (m_model) {
            const Track& track = m_model->tracks()[i];
            for (int j = 0; j < track.clips.size(); ++j) {
                QRect cr = clipRect(i, j);
                cr.translate(0, -scrollY);
                if (cr.right() < m_trackHeaderWidth || cr.left() > viewW) continue;
                cr.setLeft(qMax(cr.left(), m_trackHeaderWidth));
                cr.setRight(qMin(cr.right(), viewW));
                p.fillRect(cr, QColor(94, 129, 172));
                p.setPen(QColor(60, 90, 120));
                p.drawRect(cr.adjusted(0, 0, -1, -1));
                QFileInfo fi(track.clips[j].mediaPath);
                p.setPen(Qt::white);
                p.drawText(cr.adjusted(4, 0, -4, 0), Qt::AlignVCenter | Qt::AlignLeft,
                    p.fontMetrics().elidedText(fi.fileName(), Qt::ElideRight, cr.width() - 8));
            }
        }
    }

    // Playhead
    int playheadX = m_trackHeaderWidth + timeToPixel(m_playheadMs) - scrollX;
    if (playheadX >= m_trackHeaderWidth && playheadX < viewW) {
        p.setPen(QColor(255, 80, 80));
        p.setBrush(QColor(255, 80, 80));
        p.drawLine(playheadX, 0, playheadX, viewH);
        QPolygon tri;
        tri << QPoint(playheadX - 6, 0) << QPoint(playheadX + 6, 0) << QPoint(playheadX, 10);
        p.drawPolygon(tri);
    }
}

void TimelineWidget::contextMenuEvent(QContextMenuEvent* e) {
    const QPoint pos = e->pos();
    int contentY = pos.y() + m_vScroll->value();
    int trk = -1, clp = -1;
    findClipAt(pos.x(), contentY, &trk, &clp);
    QMenu menu(this);
    if (trk >= 0 && clp >= 0 && m_model) {
        const TimelineClip& clip = m_model->tracks()[trk].clips[clp];
        QAction* razorAct = menu.addAction(tr("Razor at playhead"));
        QAction* speedAct = menu.addAction(tr("Speed / Duration..."));
        QAction* scaleAct = menu.addAction(tr("Scale to frame size"));
        menu.addAction(tr("Unlink"));
        QAction* interpretAct = menu.addAction(tr("Interpret Footage…"));
        QAction* addEffectAct = menu.addAction(tr("Add Effect"));
        QAction* chosen = menu.exec(e->globalPos());
        if (chosen == razorAct) {
            qint64 timeMs = m_playheadMs;
            m_model->splitClipAt(trk, timeMs, nullptr, nullptr);
            emit clipSplit(trk, timeMs);
        } else if (chosen == speedAct) {
            QDialog dlg(this);
            dlg.setWindowTitle(tr("Speed / Duration"));
            QFormLayout* form = new QFormLayout(&dlg);
            QDoubleSpinBox* speedSpin = new QDoubleSpinBox(&dlg);
            speedSpin->setRange(1.0, 1000.0);
            speedSpin->setDecimals(1);
            speedSpin->setSuffix(tr("%"));
            speedSpin->setValue(clip.speedRatio * 100.0);
            form->addRow(tr("Speed:"), speedSpin);
            QDialogButtonBox* box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
            form->addRow(box);
            connect(box, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
            connect(box, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);
            if (dlg.exec() == QDialog::Accepted)
                m_model->setClipSpeedRatio(trk, clp, speedSpin->value() / 100.0);
        } else if (chosen == scaleAct)
            m_model->setClipScaleToFrame(trk, clp, true);
        else if (chosen == interpretAct)
            emit interpretFootageRequested(clip.mediaPath);
        else if (chosen == addEffectAct)
            emit addEffectRequested(trk, clp);
        if (chosen)
            update();
        return;
    }
    if (pos.x() >= m_trackHeaderWidth && contentY >= m_rulerHeight && m_model) {
        int t = trackAtY(contentY);
        if (t >= 0) {
            QAction* addVideo = menu.addAction(tr("Add Video Track"));
            QAction* addAudio = menu.addAction(tr("Add Audio Track"));
            QAction* chosen = menu.exec(e->globalPos());
            if (chosen == addVideo) {
                m_model->addTrack(true);
                emit addTrackRequested(true);
            } else if (chosen == addAudio) {
                m_model->addTrack(false);
                emit addTrackRequested(false);
            }
        }
    }
}

void TimelineWidget::mousePressEvent(QMouseEvent* e) {
    if (e->button() != Qt::LeftButton) return;
    int y = e->position().toPoint().y() + m_vScroll->value();
    int x = e->position().toPoint().x();

    if (x >= m_trackHeaderWidth) {
        qint64 timeMs = pixelToTime(x - m_trackHeaderWidth + m_hScroll->value());
        if (m_tool == Tool::Selection) {
            int trk, clp;
            findClipAt(e->position().toPoint().x(), y, &trk, &clp);
            if (trk >= 0 && clp >= 0) {
                m_dragTrack = trk;
                m_dragClip = clp;
                m_dragStartX = x;
                m_dragStartMs = m_model->tracks()[trk].clips[clp].timelineStartMs;
                emit clipSelected(trk, clp);
            } else {
                setPlayheadPositionMs(timeMs);
            }
        } else if (m_tool == Tool::Razor) {
            int trk = trackAtY(y);
            if (trk >= 0 && m_model)
                m_model->splitClipAt(trk, timeMs, nullptr, nullptr);
        } else if (m_tool == Tool::Hand) {
            // Could start pan drag
        } else {
            setPlayheadPositionMs(timeMs);
        }
    } else {
        // Click on ruler: move playhead
        if (y < m_rulerHeight)
            setPlayheadPositionMs(pixelToTime(m_hScroll->value() + x - m_trackHeaderWidth));
    }
    update();
}

void TimelineWidget::mouseMoveEvent(QMouseEvent* e) {
    if (m_dragTrack >= 0 && m_dragClip >= 0 && m_model) {
        int dx = e->position().toPoint().x() - m_dragStartX;
        qint64 dMs = static_cast<qint64>(dx / m_pixelsPerMs);
        qint64 newStart = m_dragStartMs + dMs;
        if (newStart < 0) newStart = 0;
        m_model->setClipStart(m_dragTrack, m_dragClip, newStart);
        emit clipMoved(m_dragTrack, m_dragClip, newStart);
        update();
    }
}

void TimelineWidget::mouseReleaseEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        m_dragTrack = -1;
        m_dragClip = -1;
    }
}

void TimelineWidget::wheelEvent(QWheelEvent* e) {
    if (e->modifiers() & Qt::ControlModifier) {
        if (e->angleDelta().y() > 0) zoomIn();
        else zoomOut();
        e->accept();
    } else {
        QWidget::wheelEvent(e);
    }
}

void TimelineWidget::resizeEvent(QResizeEvent* e) {
    QWidget::resizeEvent(e);
    const int zoomW = 80;
    const int bottomH = m_hScroll->height();
    if (m_zoomSlider)
        m_zoomSlider->setGeometry(0, height() - bottomH, zoomW, bottomH);
    m_hScroll->setGeometry(m_trackHeaderWidth + zoomW, height() - bottomH, width() - m_trackHeaderWidth - zoomW - m_vScroll->width(), bottomH);
    m_vScroll->setGeometry(width() - m_vScroll->width(), m_rulerHeight, m_vScroll->width(), height() - m_rulerHeight - bottomH);
    updateScrollRange();
}

} // namespace aether
