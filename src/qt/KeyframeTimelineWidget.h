#pragma once

#include <QWidget>
#include <cstdint>

namespace aether {

class KeyframeModel;

class KeyframeTimelineWidget : public QWidget {
    Q_OBJECT
public:
    explicit KeyframeTimelineWidget(QWidget* parent = nullptr);
    void setModel(KeyframeModel* model);
    void setCurrentTimeMs(int64_t timeMs);
    int64_t currentTimeMs() const { return m_currentTimeMs; }

signals:
    void currentTimeChanged(int64_t timeMs);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void updateLayout();
    int timeToPixel(int64_t ms) const;
    int64_t pixelToTime(int x) const;
    QRect keyframeRect(int paramIndex, int keyframeIndex) const;

    KeyframeModel* m_model = nullptr;
    int64_t m_currentTimeMs = 0;
    int m_pixelsPerMs = 1;
    int m_rulerHeight = 24;
    int m_rowHeight = 28;
    int m_paramNameWidth = 120;
};

} // namespace aether
