#include "NewProjectDialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFileDialog>
#include <QDir>
#include <QHBoxLayout>
#include <QMessageBox>

namespace aether {

NewProjectDialog::NewProjectDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("New Project"));
    setMinimumWidth(400);
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    m_projectNameEdit = new QLineEdit(this);
    m_projectNameEdit->setPlaceholderText(tr("Untitled Project"));
    m_projectNameEdit->setText(tr("Untitled Project"));

    m_saveLocationEdit = new QLineEdit(this);
    m_saveLocationEdit->setPlaceholderText(tr("Choose a folder..."));
    m_saveLocationEdit->setReadOnly(true);
    QPushButton* browseBtn = new QPushButton(tr("Browse..."), this);
    connect(browseBtn, &QPushButton::clicked, this, &NewProjectDialog::browseSaveLocation);
    QHBoxLayout* locationRow = new QHBoxLayout();
    locationRow->addWidget(m_saveLocationEdit, 1);
    locationRow->addWidget(browseBtn);

    m_presetCombo = new QComboBox(this);
    m_presetCombo->addItem(tr("4K (3840 × 2160)"), QVariant::fromValue<int>(0));
    m_presetCombo->addItem(tr("1080p (1920 × 1080)"), QVariant::fromValue<int>(1));
    m_presetCombo->addItem(tr("720p (1280 × 720)"), QVariant::fromValue<int>(2));
    m_presetCombo->addItem(tr("Custom"), QVariant::fromValue<int>(3));
    connect(m_presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NewProjectDialog::onPresetChanged);

    m_widthSpin = new QSpinBox(this);
    m_widthSpin->setRange(320, 7680);
    m_widthSpin->setValue(1920);
    m_heightSpin = new QSpinBox(this);
    m_heightSpin->setRange(240, 4320);
    m_heightSpin->setValue(1080);

    m_fpsCombo = new QComboBox(this);
    m_fpsCombo->addItem(tr("24 fps"), 24);
    m_fpsCombo->addItem(tr("25 fps"), 25);
    m_fpsCombo->addItem(tr("30 fps"), 30);
    m_fpsCombo->addItem(tr("60 fps"), 60);
    m_fpsCombo->setCurrentIndex(2);

    m_bitrateSpin = new QSpinBox(this);
    m_bitrateSpin->setRange(1000, 200000);
    m_bitrateSpin->setValue(25000);
    m_bitrateSpin->setSuffix(tr(" kbps"));

    m_sampleRateCombo = new QComboBox(this);
    m_sampleRateCombo->addItem(tr("44100 Hz"), 44100);
    m_sampleRateCombo->addItem(tr("48000 Hz"), 48000);
    m_sampleRateCombo->addItem(tr("96000 Hz"), 96000);
    m_sampleRateCombo->setCurrentIndex(1);

    QFormLayout* form = new QFormLayout();
    form->addRow(tr("Project name:"), m_projectNameEdit);
    form->addRow(tr("Save location:"), locationRow);
    form->addRow(tr("Preset:"), m_presetCombo);
    form->addRow(tr("Width:"), m_widthSpin);
    form->addRow(tr("Height:"), m_heightSpin);
    form->addRow(tr("Frame rate:"), m_fpsCombo);
    form->addRow(tr("Audio sample rate:"), m_sampleRateCombo);
    form->addRow(tr("Bitrate:"), m_bitrateSpin);
    mainLayout->addLayout(form);

    m_buttons = new QDialogButtonBox(this);
    QPushButton* createBtn = m_buttons->addButton(tr("Create"), QDialogButtonBox::AcceptRole);
    m_buttons->addButton(QDialogButtonBox::Cancel);
    connect(createBtn, &QPushButton::clicked, this, [this]() {
        if (m_saveLocationEdit->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, tr("New Project"), tr("Please choose a save location."));
            return;
        }
        accept();
    });
    connect(m_buttons->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &QDialog::reject);
    mainLayout->addWidget(m_buttons);

    onPresetChanged(1);
}

void NewProjectDialog::onPresetChanged(int index) {
    bool custom = (index == 3);
    m_widthSpin->setEnabled(custom);
    m_heightSpin->setEnabled(custom);
    if (!custom) {
        if (index == 0) { m_widthSpin->setValue(3840); m_heightSpin->setValue(2160); }
        else if (index == 1) { m_widthSpin->setValue(1920); m_heightSpin->setValue(1080); }
        else if (index == 2) { m_widthSpin->setValue(1280); m_heightSpin->setValue(720); }
    }
}

void NewProjectDialog::browseSaveLocation() {
    QString dir = QFileDialog::getExistingDirectory(this, tr("Choose project save location"),
        m_saveLocationEdit->text().isEmpty() ? QDir::homePath() : m_saveLocationEdit->text());
    if (!dir.isEmpty())
        m_saveLocationEdit->setText(QDir::toNativeSeparators(dir));
}

QString NewProjectDialog::projectName() const {
    QString n = m_projectNameEdit->text().trimmed();
    return n.isEmpty() ? tr("Untitled Project") : n;
}

QString NewProjectDialog::saveLocation() const {
    return m_saveLocationEdit->text().trimmed();
}

ProjectSettings NewProjectDialog::settings() const {
    ProjectSettings s;
    s.width = m_widthSpin->value();
    s.height = m_heightSpin->value();
    s.fps = m_fpsCombo->currentData().toInt();
    if (s.fps <= 0) s.fps = 30;
    s.audioSampleRate = m_sampleRateCombo->currentData().toInt();
    if (s.audioSampleRate <= 0) s.audioSampleRate = 48000;
    s.bitrateKbps = m_bitrateSpin->value();
    return s;
}

void NewProjectDialog::setSettings(const ProjectSettings& s) {
    m_widthSpin->setValue(s.width);
    m_heightSpin->setValue(s.height);
    m_bitrateSpin->setValue(s.bitrateKbps);
    for (int i = 0; i < m_fpsCombo->count(); i++) {
        if (m_fpsCombo->itemData(i).toInt() == s.fps) {
            m_fpsCombo->setCurrentIndex(i);
            break;
        }
    }
    for (int i = 0; i < m_sampleRateCombo->count(); i++) {
        if (m_sampleRateCombo->itemData(i).toInt() == s.audioSampleRate) {
            m_sampleRateCombo->setCurrentIndex(i);
            break;
        }
    }
}

} // namespace aether
