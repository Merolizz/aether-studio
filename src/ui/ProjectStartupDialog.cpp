#include "ProjectStartupDialog.h"
#include "../core/FileDialog.h"
#include "imgui.h"
#include <cstring>
#include <algorithm>

namespace aether {

ProjectStartupDialog::ProjectStartupDialog() {
    std::memset(m_projectNameBuffer, 0, sizeof(m_projectNameBuffer));
}

void ProjectStartupDialog::show() {
    m_visible = true;
    m_done = false;
    m_result = DialogResult::None;
    m_selectedTab = 0;
    std::memset(m_projectNameBuffer, 0, sizeof(m_projectNameBuffer));
    m_settings = ProjectSettings{};
}

void ProjectStartupDialog::close() {
    m_visible = false;
}

void ProjectStartupDialog::render() {
    if (!m_visible) {
        return;
    }

    // Center the dialog
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Project Startup", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {
        
        // Tabs
        if (ImGui::BeginTabBar("ProjectTabs")) {
            if (ImGui::BeginTabItem("New Project")) {
                m_selectedTab = 0;
                renderNewProjectTab();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Open Project")) {
                m_selectedTab = 1;
                renderOpenProjectTab();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Buttons
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 220);
        
        if (m_selectedTab == 0) {
            // New Project buttons
            if (ImGui::Button("Cancel", ImVec2(100, 0))) {
                m_result = DialogResult::Cancel;
                m_done = true;
                m_visible = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            
            bool canCreate = validateNewProject();
            if (!canCreate) {
                ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
            }
            
            if (ImGui::Button("Create", ImVec2(100, 0))) {
                if (validateNewProject()) {
                    m_settings.projectName = m_projectNameBuffer;
                    
                    // Set resolution
                    if (m_resolutionIndex == 0) {
                        m_settings.width = 1920;
                        m_settings.height = 1080;
                    } else if (m_resolutionIndex == 1) {
                        m_settings.width = 3840;
                        m_settings.height = 2160;
                    } else if (m_resolutionIndex == 2) {
                        m_settings.width = 7680;
                        m_settings.height = 4320;
                    } else {
                        m_settings.width = m_customWidth;
                        m_settings.height = m_customHeight;
                    }
                    
                    // Set FPS
                    if (m_fpsIndex == 0) {
                        m_settings.fps = 24;
                    } else if (m_fpsIndex == 1) {
                        m_settings.fps = 25;
                    } else if (m_fpsIndex == 2) {
                        m_settings.fps = 30;
                    } else if (m_fpsIndex == 3) {
                        m_settings.fps = 50;
                    } else if (m_fpsIndex == 4) {
                        m_settings.fps = 60;
                    } else if (m_fpsIndex == 5) {
                        m_settings.fps = 120;
                    } else {
                        m_settings.fps = m_customFps;
                    }
                    
                    m_result = DialogResult::NewProject;
                    m_done = true;
                    m_visible = false;
                    ImGui::CloseCurrentPopup();
                }
            }
            
            if (!canCreate) {
                ImGui::PopItemFlag();
                ImGui::PopStyleVar();
            }
        } else {
            // Open Project buttons
            if (ImGui::Button("Cancel", ImVec2(100, 0))) {
                m_result = DialogResult::Cancel;
                m_done = true;
                m_visible = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            
            if (ImGui::Button("Browse...", ImVec2(100, 0))) {
                std::vector<std::pair<std::string, std::string>> filters = {
                    {"Aether Studio Projects", "*.aether"},
                    {"All Files", "*.*"}
                };
                std::string filePath = FileDialog::openFile("Open Project", filters);
                if (!filePath.empty()) {
                    m_settings.projectPath = filePath;
                    // Extract project name from path
                    size_t lastSlash = filePath.find_last_of("\\/");
                    size_t lastDot = filePath.find_last_of(".");
                    if (lastSlash != std::string::npos && lastDot != std::string::npos && lastDot > lastSlash) {
                        m_settings.projectName = filePath.substr(lastSlash + 1, lastDot - lastSlash - 1);
                    } else {
                        m_settings.projectName = filePath;
                    }
                }
            }
            ImGui::SameLine();
            
            if (ImGui::Button("Open", ImVec2(100, 0))) {
                if (!m_settings.projectPath.empty()) {
                    m_result = DialogResult::OpenProject;
                    m_done = true;
                    m_visible = false;
                    ImGui::CloseCurrentPopup();
                }
            }
        }

        ImGui::EndPopup();
    } else {
        // Popup was closed
        if (m_visible) {
            m_result = DialogResult::Cancel;
            m_done = true;
            m_visible = false;
        }
    }

    // Open popup if visible
    if (m_visible && !ImGui::IsPopupOpen("Project Startup")) {
        ImGui::OpenPopup("Project Startup");
    }
}

void ProjectStartupDialog::renderNewProjectTab() {
    ImGui::Spacing();

    // Project Name
    ImGui::Text("Project Name:");
    ImGui::PushItemWidth(-1);
    ImGui::InputText("##ProjectName", m_projectNameBuffer, sizeof(m_projectNameBuffer));
    ImGui::PopItemWidth();

    ImGui::Spacing();

    // Resolution
    ImGui::Text("Resolution:");
    ImGui::PushItemWidth(-1);
    if (ImGui::Combo("##Resolution", &m_resolutionIndex, RESOLUTION_OPTIONS, IM_ARRAYSIZE(RESOLUTION_OPTIONS))) {
        m_showCustomResolution = (m_resolutionIndex == 3);
    }
    ImGui::PopItemWidth();

    if (m_showCustomResolution) {
        ImGui::Spacing();
        ImGui::Text("Custom Resolution:");
        ImGui::PushItemWidth(100);
        ImGui::InputScalar("Width##CustomRes", ImGuiDataType_U32, &m_customWidth, nullptr, nullptr, "%u");
        ImGui::SameLine();
        ImGui::Text("x");
        ImGui::SameLine();
        ImGui::InputScalar("Height##CustomRes", ImGuiDataType_U32, &m_customHeight, nullptr, nullptr, "%u");
        ImGui::PopItemWidth();
        
        if (m_customWidth == 0) m_customWidth = 1;
        if (m_customHeight == 0) m_customHeight = 1;
    }

    ImGui::Spacing();

    // FPS
    ImGui::Text("Frame Rate (FPS):");
    ImGui::PushItemWidth(-1);
    if (ImGui::Combo("##FPS", &m_fpsIndex, FPS_OPTIONS, IM_ARRAYSIZE(FPS_OPTIONS))) {
        m_showCustomFps = (m_fpsIndex == 6);
    }
    ImGui::PopItemWidth();

    if (m_showCustomFps) {
        ImGui::Spacing();
        ImGui::Text("Custom FPS:");
        ImGui::PushItemWidth(100);
        ImGui::InputScalar("##CustomFPS", ImGuiDataType_U32, &m_customFps, nullptr, nullptr, "%u");
        ImGui::PopItemWidth();
        
        if (m_customFps == 0) m_customFps = 1;
    }
}

void ProjectStartupDialog::renderOpenProjectTab() {
    ImGui::Spacing();
    ImGui::TextWrapped("Open an existing Aether Studio project file.");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Project path display
    ImGui::Text("Project Path:");
    ImGui::PushItemWidth(-1);
    char pathBuffer[512] = {0};
    if (!m_settings.projectPath.empty()) {
        strncpy_s(pathBuffer, sizeof(pathBuffer), m_settings.projectPath.c_str(), _TRUNCATE);
    }
    ImGui::InputText("##ProjectPath", pathBuffer, sizeof(pathBuffer), ImGuiInputTextFlags_ReadOnly);
    ImGui::PopItemWidth();
    
    if (!m_settings.projectPath.empty()) {
        m_settings.projectPath = pathBuffer;
    }
    
    ImGui::Spacing();
    ImGui::Text("Recent Projects:");
    ImGui::Spacing();
    ImGui::Text("(No recent projects)");
}

bool ProjectStartupDialog::validateNewProject() {
    if (std::strlen(m_projectNameBuffer) == 0) {
        return false;
    }

    uint32_t width, height, fps;

    if (m_resolutionIndex == 0) {
        width = 1920;
        height = 1080;
    } else if (m_resolutionIndex == 1) {
        width = 3840;
        height = 2160;
    } else if (m_resolutionIndex == 2) {
        width = 7680;
        height = 4320;
    } else {
        width = m_customWidth;
        height = m_customHeight;
    }

    if (m_fpsIndex == 0) {
        fps = 24;
    } else if (m_fpsIndex == 1) {
        fps = 25;
    } else if (m_fpsIndex == 2) {
        fps = 30;
    } else if (m_fpsIndex == 3) {
        fps = 50;
    } else if (m_fpsIndex == 4) {
        fps = 60;
    } else if (m_fpsIndex == 5) {
        fps = 120;
    } else {
        fps = m_customFps;
    }

    return width > 0 && height > 0 && fps > 0;
}

} // namespace aether
