#include "StatusBar.h"
#include "../../include/aether/ProjectContext.h"
#include "../../include/aether/HardwareOrchestrator.h"
#include "../../include/aether/MemoryManager.h"
#include "imgui.h"
#include <sstream>

namespace aether {

StatusBar::StatusBar() {
}

void StatusBar::render() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - 20));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 20));
    
    ImGui::Begin("StatusBar", nullptr, 
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus);
    
    auto& context = ProjectContext::getInstance();
    
    // Project info
    if (context.isProjectOpen()) {
        renderProjectInfo();
        ImGui::SameLine();
        ImGui::Separator();
        ImGui::SameLine();
    }
    
    // System info
    renderSystemInfo();
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    
    // Memory info
    renderMemoryInfo();
    ImGui::SameLine();
    ImGui::Separator();
    ImGui::SameLine();
    
    // Render mode
    renderRenderMode();
    
    ImGui::End();
}

void StatusBar::renderProjectInfo() {
    auto& context = ProjectContext::getInstance();
    
    if (context.isProjectOpen()) {
        const auto& settings = context.getSettings();
        std::stringstream ss;
        ss << "Project: " << settings.projectName;
        ss << " | " << settings.width << "x" << settings.height;
        ss << " @ " << settings.fps << "fps";
        
        if (context.hasUnsavedChanges()) {
            ss << " *";
        }
        
        ImGui::Text("%s", ss.str().c_str());
    } else {
        ImGui::Text("No project open");
    }
}

void StatusBar::renderSystemInfo() {
    auto& hardwareOrchestrator = HardwareOrchestrator::getInstance();
    
    if (hardwareOrchestrator.getAvailableGPUs().size() > 0) {
        const auto& gpu = hardwareOrchestrator.getPrimaryGPU();
        ImGui::Text("GPU: %s", gpu.name.c_str());
    } else {
        ImGui::Text("GPU: Software");
    }
}

void StatusBar::renderMemoryInfo() {
    auto& memoryManager = MemoryManager::getInstance();
    
    uint64_t usedMB = memoryManager.getUsedRAM() / (1024 * 1024);
    uint64_t totalMB = memoryManager.getTotalRAM() / (1024 * 1024);
    float percentage = memoryManager.getRAMUsagePercentage();
    
    ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    if (memoryManager.isOverLimit()) {
        color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f); // Red if over limit
    } else if (percentage > 60.0f) {
        color = ImVec4(1.0f, 0.8f, 0.3f, 1.0f); // Yellow if > 60%
    }
    
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::Text("RAM: %llu/%llu MB (%.1f%%)", usedMB, totalMB, percentage);
    ImGui::PopStyleColor();
}

void StatusBar::renderRenderMode() {
    auto& hardwareOrchestrator = HardwareOrchestrator::getInstance();
    auto renderMode = hardwareOrchestrator.getCurrentRenderMode();
    
    const char* modeName = "Unknown";
    switch (renderMode) {
        case RenderMode::Vulkan: modeName = "Vulkan"; break;
        case RenderMode::CUDA: modeName = "CUDA"; break;
        case RenderMode::OpenCL: modeName = "OpenCL"; break;
        case RenderMode::QuickSync: modeName = "QuickSync"; break;
        case RenderMode::Software: modeName = "Software"; break;
    }
    
    ImGui::Text("Render: %s", modeName);
}

} // namespace aether
