#pragma once

#include <QWidget>

namespace aether {

class ColorWheelsWidget;
class VideoScopesWidget;

class ColorPageWidget : public QWidget {
    Q_OBJECT
public:
    explicit ColorPageWidget(QWidget* parent = nullptr);

private:
    ColorWheelsWidget* m_wheels = nullptr;
    VideoScopesWidget* m_scopes = nullptr;
};

} // namespace aether
