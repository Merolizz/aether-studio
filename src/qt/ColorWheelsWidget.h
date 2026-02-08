#pragma once

#include <QWidget>
#include "aether/ColorGradeState.h"

namespace aether {

class ColorWheelsWidget : public QWidget {
    Q_OBJECT
public:
    explicit ColorWheelsWidget(QWidget* parent = nullptr);
    void getState(ColorGradeState* out) const;
    void setState(const ColorGradeState& state);

signals:
    void stateChanged();

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;

private:
    ColorGradeState m_state;
};

} // namespace aether
