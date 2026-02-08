#pragma once

#include <QWidget>
#include <QImage>

namespace aether {

class VideoScopesWidget : public QWidget {
    Q_OBJECT
public:
    explicit VideoScopesWidget(QWidget* parent = nullptr);
    void setFrame(const QImage& frame);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QImage m_frame;
};

} // namespace aether
