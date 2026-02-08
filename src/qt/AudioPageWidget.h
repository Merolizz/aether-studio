#pragma once

#include <QWidget>

namespace aether {

class AudioPageWidget : public QWidget {
    Q_OBJECT
public:
    explicit AudioPageWidget(QWidget* parent = nullptr);
};

} // namespace aether
