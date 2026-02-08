#pragma once

#include "aether/ProjectSettings.h"
#include <QDialog>

class QComboBox;
class QSpinBox;
class QDialogButtonBox;
class QLineEdit;

namespace aether {

class NewProjectDialog : public QDialog {
    Q_OBJECT
public:
    explicit NewProjectDialog(QWidget* parent = nullptr);
    ProjectSettings settings() const;
    void setSettings(const ProjectSettings& s);
    QString projectName() const;
    QString saveLocation() const;

private:
    void onPresetChanged(int index);
    void browseSaveLocation();

    QLineEdit* m_projectNameEdit = nullptr;
    QLineEdit* m_saveLocationEdit = nullptr;
    QComboBox* m_presetCombo = nullptr;
    QSpinBox* m_widthSpin = nullptr;
    QSpinBox* m_heightSpin = nullptr;
    QComboBox* m_fpsCombo = nullptr;
    QComboBox* m_sampleRateCombo = nullptr;
    QSpinBox* m_bitrateSpin = nullptr;
    QDialogButtonBox* m_buttons = nullptr;
};

} // namespace aether
