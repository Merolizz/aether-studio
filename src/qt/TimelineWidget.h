#pragma once

#include <QWidget>
#include <QPointer>
#include <QScrollBar>

class QSlider;

namespace aether {

class ProjectModel;

class TimelineWidget : public QWidget {
    Q_OBJECT
public:
    explicit TimelineWidget(ProjectModel* model, QWidget* parent = nullptr);

    void setModel(ProjectModel* model);
    qint64 playheadPositionMs() const { return m_playheadMs; }
    void setPlayheadPositionMs(qint64 ms);
    qint64 pixelsPerMs() const { return m_pixelsPerMs; }
    void setPixelsPerMs(double ppm);
    void zoomIn();
    void zoomOut();
    void zoomFit();
    int trackHeight() const { return m_trackHeight; }
    void setTrackHeight(int h);

    enum class Tool { Selection, Razor, Slip, Slide, Pen, Hand, Zoom };
    Tool currentTool() const { return m_tool; }
    void setTool(Tool t);

signals:
    void playheadMoved(qint64 ms);
    void clipSelected(int trackIndex, int clipIndex);
    void clipMoved(int trackIndex, int clipIndex, qint64 newStartMs);
    void clipSplit(int trackIndex, qint64 positionMs);
    void addClipRequested(const QString& mediaPath, qint64 atMs);
    void addTrackRequested(bool isVideo);
    void addEffectRequested(int trackIndex, int clipIndex);
    void interpretFootageRequested(const QString& mediaPath);

private:
    void paintEvent(QPaintEvent*) override;
    void contextMenuEvent(QContextMenuEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void resizeEvent(QResizeEvent*) override;

    int timeToPixel(qint64 ms) const;
    qint64 pixelToTime(int x) const;
    void updateScrollRange();
    QRect trackHeaderRect(int trackIndex) const;
    QRect trackContentRect(int trackIndex) const;
    QRect clipRect(int trackIndex, int clipIndex) const;
    int trackAtY(int y) const;
    void findClipAt(int x, int y, int* outTrack, int* outClip) const;

    QPointer<ProjectModel> m_model;
    qint64 m_playheadMs = 0;
    double m_pixelsPerMs = 0.1;
    int m_trackHeight = 48;
    int m_rulerHeight = 24;
    int m_trackHeaderWidth = 100;
    Tool m_tool = Tool::Selection;
    QScrollBar* m_hScroll = nullptr;
    QScrollBar* m_vScroll = nullptr;
    class QSlider* m_zoomSlider = nullptr;
    int m_contentWidth = 0;
    int m_contentHeight = 0;
    int m_dragTrack = -1;
    int m_dragClip = -1;
    int m_dragStartX = 0;
    qint64 m_dragStartMs = 0;
};

} // namespace aether
