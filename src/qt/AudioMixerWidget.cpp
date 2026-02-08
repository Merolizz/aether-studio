#include "AudioMixerWidget.h"
#include <QVBoxLayout>
#include <QLabel>

namespace aether {

AudioMixerWidget::AudioMixerWidget(QWidget* parent) : QWidget(parent) {
    setStyleSheet("AudioMixerWidget { background: #1e1e1e; }");
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(tr("Channel Mixer"), this));
}

} // namespace aether
