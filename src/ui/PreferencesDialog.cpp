#include "PreferencesDialog.h"
#include "../../include/aether/KeymapManager.h"
#include "../../include/aether/HardwareOrchestrator.h"
#include "../../include/aether/WorkspaceManager.h"
#include "imgui.h"
#include <iostream>

namespace aether {

PreferencesDialog::PreferencesDialog() {
}

void PreferencesDialog::show() {
    m_visible = true;
    m_selectedTab = 0;
}

void PreferencesDialog::close() {
    m_visible = false;
}

void PreferencesDialog::render() {
    if (!m_visible) {
        return;
    }

    // Center the dialog
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Preferences", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {
        
        // Tabs
        if (ImGui::BeginTabBar("PreferencesTabs")) {
            if (ImGui::BeginTabItem("General")) {
                m_selectedTab = 0;
                renderGeneralTab();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Display")) {
                m_selectedTab = 1;
                renderDisplayTab();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Performance")) {
                m_selectedTab = 2;
                renderPerformanceTab();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Keymap")) {
                m_selectedTab = 3;
                renderKeymapTab();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Buttons
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 100);
        
        if (ImGui::Button("Close", ImVec2(100, 0))) {
            m_visible = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    } else {
        if (m_visible) {
            m_visible = false;
        }
    }

    // Open popup if visible
    if (m_visible && !ImGui::IsPopupOpen("Preferences")) {
        ImGui::OpenPopup("Preferences");
    }
}

void PreferencesDialog::renderGeneralTab() {
    ImGui::Spacing();
    ImGui::Text("General Settings");
    ImGui::Spacing();
    
    // Auto-save
    static bool autoSave = false;
    ImGui::Checkbox("Auto-save projects", &autoSave);
    
    // Auto-save interval
    static int autoSaveInterval = 5;
    ImGui::SliderInt("Auto-save interval (minutes)", &autoSaveInterval, 1, 60);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Language
    ImGui::Text("Language:");
    static int language = 0;
    const char* languages[] = { "English", "Turkish", "German", "French" };
    ImGui::Combo("##Language", &language, languages, IM_ARRAYSIZE(languages));
}

void PreferencesDialog::renderDisplayTab() {
    ImGui::Spacing();
    ImGui::Text("Display Settings");
    ImGui::Spacing();
    
    // UI Scale
    static float uiScale = 1.0f;
    ImGui::SliderFloat("UI Scale", &uiScale, 0.5f, 2.0f);
    
    ImGui::Spacing();
    
    // Theme
    ImGui::Text("Theme:");
    static int theme = 0;
    const char* themes[] = { "Dark", "Light", "Classic" };
    ImGui::Combo("##Theme", &theme, themes, IM_ARRAYSIZE(themes));
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Show tooltips
    static bool showTooltips = true;
    ImGui::Checkbox("Show tooltips", &showTooltips);
    
    // Show status bar
    static bool showStatusBar = true;
    ImGui::Checkbox("Show status bar", &showStatusBar);
}

void PreferencesDialog::renderPerformanceTab() {
    ImGui::Spacing();
    ImGui::Text("Performance Settings");
    ImGui::Spacing();
    
    auto& hardwareOrchestrator = HardwareOrchestrator::getInstance();
    const auto& gpu = hardwareOrchestrator.getPrimaryGPU();
    
    ImGui::Text("GPU: %s", gpu.name.c_str());
    ImGui::Text("Driver: %s", gpu.driverVersion.c_str());
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Render mode
    ImGui::Text("Render Mode:");
    auto currentMode = hardwareOrchestrator.getCurrentRenderMode();
    const char* modes[] = { "Vulkan", "CUDA", "OpenCL", "QuickSync", "Software" };
    int modeIndex = static_cast<int>(currentMode);
    if (ImGui::Combo("##RenderMode", &modeIndex, modes, IM_ARRAYSIZE(modes))) {
        hardwareOrchestrator.setRenderMode(static_cast<RenderMode>(modeIndex));
    }
    
    ImGui::Spacing();
    
    // Memory limit
    static int memoryLimit = 8192; // MB
    ImGui::SliderInt("Memory Limit (MB)", &memoryLimit, 1024, 32768);
}

void PreferencesDialog::renderKeymapTab() {
    ImGui::Spacing();
    ImGui::Text("Keyboard Shortcuts");
    ImGui::Spacing();
    
    auto& keymapManager = KeymapManager::getInstance();
    auto bindings = keymapManager.getAllBindings();
    
    if (ImGui::BeginTable("KeymapTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Shortcut", ImGuiTableColumnFlags_WidthFixed, 200);
        ImGui::TableSetupColumn("Context", ImGuiTableColumnFlags_WidthFixed, 100);
        ImGui::TableHeadersRow();
        
        for (const auto& binding : bindings) {
            ImGui::TableNextRow();
            
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", binding.action.c_str());
            
            ImGui::TableSetColumnIndex(1);
            std::string shortcut = keymapManager.getKeyBindingString(binding.action, binding.context);
            ImGui::Text("%s", shortcut.c_str());
            
            ImGui::TableSetColumnIndex(2);
            const char* contextName = "Global";
            switch (binding.context) {
                case WorkspaceContext::Edit: contextName = "Edit"; break;
                case WorkspaceContext::Animation: contextName = "Animation"; break;
                case WorkspaceContext::Photo: contextName = "Photo"; break;
                case WorkspaceContext::Color: contextName = "Color"; break;
                case WorkspaceContext::Audio: contextName = "Audio"; break;
                default: break;
            }
            ImGui::Text("%s", contextName);
        }
        
        ImGui::EndTable();
    }
    
    ImGui::Spacing();
    
    if (ImGui::Button("Reset to Defaults")) {
        keymapManager.loadDefaultKeymap();
    }
    
    ImGui::SameLine();
    
    if (ImGui::Button("Export Keymap...")) {
        std::string keymapPath = keymapManager.getKeymapFilePath();
        keymapManager.saveKeymap(keymapPath);
    }
}

} // namespace aether
