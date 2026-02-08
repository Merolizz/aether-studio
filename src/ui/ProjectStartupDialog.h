#pragma once

#include "../../include/aether/ProjectSettings.h"
#include <string>

namespace aether {

class ProjectStartupDialog {
public:
    ProjectStartupDialog();
    ~ProjectStartupDialog() = default;

    ProjectStartupDialog(const ProjectStartupDialog&) = delete;
    ProjectStartupDialog& operator=(const ProjectStartupDialog&) = delete;

    void show();
    void close();
    void render();

    bool isDone() const { return m_done; }
    DialogResult getResult() const { return m_result; }
    ProjectSettings getSettings() const { return m_settings; }

private:
    void renderNewProjectTab();
    void renderOpenProjectTab();
    bool validateNewProject();

    bool m_visible = false;
    bool m_done = false;
    DialogResult m_result = DialogResult::None;
    ProjectSettings m_settings;

    // UI state
    int m_selectedTab = 0; // 0 = New Project, 1 = Open Project
    char m_projectNameBuffer[256] = "";
    int m_resolutionIndex = 0; // 0 = 1920x1080, 1 = 3840x2160, 2 = 7680x4320, 3 = Custom
    int m_fpsIndex = 2; // 0 = 24, 1 = 25, 2 = 30, 3 = 50, 4 = 60, 5 = 120, 6 = Custom
    uint32_t m_customWidth = 1920;
    uint32_t m_customHeight = 1080;
    uint32_t m_customFps = 30;
    bool m_showCustomResolution = false;
    bool m_showCustomFps = false;
    
    // Open project path
    std::string m_openProjectPath;

    static constexpr const char* RESOLUTION_OPTIONS[] = {
        "1920x1080 (Full HD)",
        "3840x2160 (4K UHD)",
        "7680x4320 (8K UHD)",
        "Custom"
    };

    static constexpr const char* FPS_OPTIONS[] = {
        "24",
        "25",
        "30",
        "50",
        "60",
        "120",
        "Custom"
    };
};

} // namespace aether
