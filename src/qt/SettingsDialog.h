#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;
class QCheckBox;
class QComboBox;

namespace aether {

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override = default;

private:
    void loadSettings();
    void saveSettings();
    void browseFfmpeg();

    QLineEdit* m_ffmpegPath = nullptr;
    QCheckBox* m_skipFfmpegCheck = nullptr;
    QLineEdit* m_proxyDir = nullptr;
    QCheckBox* m_useProxyForPlayback = nullptr;
    QComboBox* m_playbackResolution = nullptr;
    QComboBox* m_performanceProfile = nullptr;
};

} // namespace aether
