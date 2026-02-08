#pragma once

#include <QWidget>

namespace aether {

class AudioMixerWidget : public QWidget {
    Q_OBJECT
public:
    explicit AudioMixerWidget(QWidget* parent = nullptr);
};

} // namespace aether
