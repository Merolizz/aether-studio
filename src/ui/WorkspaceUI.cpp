#include "WorkspaceUI.h"
#include "Toolbar.h"
#include "../../include/aether/WorkspaceManager.h"
#include "imgui.h"
#include <iostream>
#include <memory>

namespace aether {

WorkspaceUI::WorkspaceUI() {
    m_toolbar = std::make_unique<Toolbar>();
}

void WorkspaceUI::render() {
    renderWorkspaceTabs();
    
    // Render toolbar
    if (m_toolbar) {
        auto& workspaceManager = WorkspaceManager::getInstance();
        m_toolbar->setWorkspace(workspaceManager.getCurrentWorkspace());
        m_toolbar->render();
    }
    
    renderWorkspaceContent();
}

void WorkspaceUI::renderWorkspaceTabs() {
    auto& workspaceManager = WorkspaceManager::getInstance();
    auto availableWorkspaces = workspaceManager.getAvailableWorkspaces();
    auto currentWorkspace = workspaceManager.getCurrentWorkspace();
    
    if (ImGui::BeginTabBar("WorkspaceTabs", ImGuiTabBarFlags_None)) {
        for (auto workspaceType : availableWorkspaces) {
            const std::string& name = workspaceManager.getWorkspaceName(workspaceType);
            bool isSelected = (workspaceType == currentWorkspace);
            
            ImGuiTabItemFlags flags = isSelected ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None;
            
            if (ImGui::BeginTabItem(name.c_str(), nullptr, flags)) {
                if (!isSelected) {
                    workspaceManager.switchToWorkspace(workspaceType);
                }
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
}

void WorkspaceUI::renderWorkspaceContent() {
    auto& workspaceManager = WorkspaceManager::getInstance();
    auto* currentWorkspace = workspaceManager.getCurrentWorkspaceInstance();
    
    if (!currentWorkspace) {
        ImGui::Text("No workspace selected");
        return;
    }
    
    // Render workspace-specific content
    // This will be overridden by each workspace's render() method
    ImGui::BeginChild("WorkspaceContent", ImVec2(0, 0), true);
    
    // Workspace will render its own content
    currentWorkspace->render();
    
    ImGui::EndChild();
}

} // namespace aether
