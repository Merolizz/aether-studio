#include "MainMenuBar.h"
#include "../../include/aether/Account.h"
#include "../../include/aether/Clipboard.h"
#include "../../include/aether/ProjectContext.h"
#include "../../include/aether/UndoRedo.h"
#include "../../include/aether/WorkspaceManager.h"
#include "../core/FileDialog.h"
#include "LoginDialog.h"
#include "imgui.h"
#include <iostream>


namespace aether {

MainMenuBar::MainMenuBar() {}

void MainMenuBar::render() {
  if (ImGui::BeginMainMenuBar()) {
    renderFileMenu();
    renderEditMenu();
    renderViewMenu();
    renderWorkspaceMenu();
    renderHelpMenu();

    ImGui::EndMainMenuBar();
  }

  // About dialog
  if (m_showAbout) {
    ImGui::OpenPopup("About Aether Studio");
    m_showAbout = false;
  }

  if (ImGui::BeginPopupModal("About Aether Studio", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Aether Studio");
    ImGui::Text("Version 1.0.0");
    ImGui::Separator();
    ImGui::Text("Professional Creative Suite");
    ImGui::Text("Edit, Animation, Photo, Color, Audio");
    ImGui::Spacing();
    if (ImGui::Button("Close")) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void MainMenuBar::renderFileMenu() {
  if (ImGui::BeginMenu("File")) {
    auto &context = ProjectContext::getInstance();

    if (ImGui::MenuItem("New Project...", "Ctrl+N")) {
      // Trigger new project dialog
    }

    if (ImGui::MenuItem("Open Project...", "Ctrl+O")) {
      std::vector<std::pair<std::string, std::string>> filters = {
          {"Aether Studio Projects", "*.aether"}, {"All Files", "*.*"}};
      std::string filePath = FileDialog::openFile("Open Project", filters);
      if (!filePath.empty()) {
        context.loadProject(filePath);
      }
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Save Project", "Ctrl+S", false,
                        context.isProjectOpen())) {
      context.saveProject();
    }

    if (ImGui::MenuItem("Save Project As...", "Ctrl+Shift+S", false,
                        context.isProjectOpen())) {
      std::vector<std::pair<std::string, std::string>> filters = {
          {"Aether Studio Projects", "*.aether"}};
      std::string filePath = FileDialog::saveFile(
          "Save Project", filters, "", context.getProjectName() + ".aether");
      if (!filePath.empty()) {
        context.saveProjectAs(filePath);
      }
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Close Project", nullptr, false,
                        context.isProjectOpen())) {
      context.closeProject();
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Exit", "Alt+F4")) {
      // Will be handled by window close
    }

    ImGui::EndMenu();
  }
}

void MainMenuBar::renderEditMenu() {
  if (ImGui::BeginMenu("Edit")) {
    auto &undoRedo = UndoRedoManager::getInstance();

    std::string undoText = "Undo";
    if (undoRedo.canUndo()) {
      undoText += " " + undoRedo.getUndoDescription();
    }
    if (ImGui::MenuItem(undoText.c_str(), "Ctrl+Z", false,
                        undoRedo.canUndo())) {
      undoRedo.undo();
    }

    std::string redoText = "Redo";
    if (undoRedo.canRedo()) {
      redoText += " " + undoRedo.getRedoDescription();
    }
    if (ImGui::MenuItem(redoText.c_str(), "Ctrl+Shift+Z", false,
                        undoRedo.canRedo())) {
      undoRedo.redo();
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Cut", "Ctrl+X")) {
      auto &clipboard = Clipboard::getInstance();
      // In a real implementation, this would cut selected content
      // For now, just clear clipboard
      clipboard.clear();
    }

    if (ImGui::MenuItem("Copy", "Ctrl+C")) {
      auto &clipboard = Clipboard::getInstance();
      // In a real implementation, this would copy selected content
      // For now, just set empty text
      clipboard.setText("");
    }

    auto &clipboard = Clipboard::getInstance();
    if (ImGui::MenuItem("Paste", "Ctrl+V", false, clipboard.hasText())) {
      std::string text = clipboard.getText();
      // In a real implementation, this would paste content
      std::cout << "Pasting: " << text << std::endl;
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Preferences...", "Ctrl+,")) {
      m_showPreferences = true;
    }

    ImGui::EndMenu();
  }
}

void MainMenuBar::renderViewMenu() {
  if (ImGui::BeginMenu("View")) {
    if (ImGui::MenuItem("Zoom In", "Ctrl++")) {
      // TODO: Implement zoom
    }

    if (ImGui::MenuItem("Zoom Out", "Ctrl+-")) {
      // TODO: Implement zoom
    }

    if (ImGui::MenuItem("Fit to Window", "Ctrl+0")) {
      // TODO: Implement fit to window
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Fullscreen", "F11")) {
      // TODO: Toggle fullscreen
    }

    ImGui::EndMenu();
  }
}

void MainMenuBar::renderWorkspaceMenu() {
  if (ImGui::BeginMenu("Workspace")) {
    auto &workspaceManager = WorkspaceManager::getInstance();
    auto currentWorkspace = workspaceManager.getCurrentWorkspace();

    if (ImGui::MenuItem("Edit", nullptr,
                        currentWorkspace == WorkspaceType::Edit)) {
      workspaceManager.switchToWorkspace(WorkspaceType::Edit);
    }

    if (ImGui::MenuItem("Animation", nullptr,
                        currentWorkspace == WorkspaceType::Animation)) {
      workspaceManager.switchToWorkspace(WorkspaceType::Animation);
    }

    if (ImGui::MenuItem("Photo", nullptr,
                        currentWorkspace == WorkspaceType::Photo)) {
      workspaceManager.switchToWorkspace(WorkspaceType::Photo);
    }

    if (ImGui::MenuItem("Color", nullptr,
                        currentWorkspace == WorkspaceType::Color)) {
      workspaceManager.switchToWorkspace(WorkspaceType::Color);
    }

    if (ImGui::MenuItem("Audio", nullptr,
                        currentWorkspace == WorkspaceType::Audio)) {
      workspaceManager.switchToWorkspace(WorkspaceType::Audio);
    }

    if (ImGui::MenuItem("Deliver", nullptr,
                        currentWorkspace == WorkspaceType::Deliver)) {
      workspaceManager.switchToWorkspace(WorkspaceType::Deliver);
    }

    ImGui::EndMenu();
  }
}

void MainMenuBar::renderHelpMenu() {
  if (ImGui::BeginMenu("Help")) {
    auto &accountManager = AccountManager::getInstance();
    (void)accountManager; // Reserved for future account status display

    if (ImGui::MenuItem("Account & License...")) {
      // TODO: Show login dialog
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Keyboard Shortcuts...")) {
      // TODO: Show keymap editor
    }

    ImGui::Separator();

    if (ImGui::MenuItem("About Aether Studio...")) {
      m_showAbout = true;
    }

    ImGui::EndMenu();
  }
}

} // namespace aether
