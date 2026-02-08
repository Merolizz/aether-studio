#include "AudioPageWidget.h"
#include "AudioMixerWidget.h"
#include <QVBoxLayout>
#include <QLabel>

namespace aether {

AudioPageWidget::AudioPageWidget(QWidget* parent) : QWidget(parent) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Audio — Mixer & VU"), this));
    layout->addWidget(new AudioMixerWidget(this), 1);
}

} // namespace aether
