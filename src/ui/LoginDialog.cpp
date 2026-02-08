#include "LoginDialog.h"
#include "../../include/aether/Account.h"
#include "imgui.h"
#include <cstring>

namespace aether {

LoginDialog::LoginDialog() {
    std::memset(m_usernameBuffer, 0, sizeof(m_usernameBuffer));
    std::memset(m_passwordBuffer, 0, sizeof(m_passwordBuffer));
    std::memset(m_emailBuffer, 0, sizeof(m_emailBuffer));
    std::memset(m_activationCodeBuffer, 0, sizeof(m_activationCodeBuffer));
}

void LoginDialog::show() {
    m_visible = true;
    m_done = false;
    m_showActivation = false;
    m_selectedTab = 0;
    m_loginError = false;
    m_registerError = false;
    m_activationError = false;
    m_errorMessage.clear();
    
    std::memset(m_usernameBuffer, 0, sizeof(m_usernameBuffer));
    std::memset(m_passwordBuffer, 0, sizeof(m_passwordBuffer));
    std::memset(m_emailBuffer, 0, sizeof(m_emailBuffer));
    std::memset(m_activationCodeBuffer, 0, sizeof(m_activationCodeBuffer));
}

void LoginDialog::close() {
    m_visible = false;
}

void LoginDialog::render() {
    if (!m_visible) {
        return;
    }

    // Center the dialog
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 center = ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 350), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Account & License", nullptr, 
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {
        
        // Tabs
        if (ImGui::BeginTabBar("AccountTabs")) {
            if (ImGui::BeginTabItem("Login")) {
                m_selectedTab = 0;
                renderLoginTab();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Register")) {
                m_selectedTab = 1;
                renderRegisterTab();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Activate License")) {
                m_selectedTab = 2;
                renderActivationTab();
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
            m_done = true;
            m_visible = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    } else {
        if (m_visible) {
            m_done = true;
            m_visible = false;
        }
    }

    // Open popup if visible
    if (m_visible && !ImGui::IsPopupOpen("Account & License")) {
        ImGui::OpenPopup("Account & License");
    }
}

void LoginDialog::renderLoginTab() {
    ImGui::Spacing();

    if (m_loginError) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::TextWrapped("%s", m_errorMessage.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    ImGui::Text("Username:");
    ImGui::PushItemWidth(-1);
    ImGui::InputText("##Username", m_usernameBuffer, sizeof(m_usernameBuffer));
    ImGui::PopItemWidth();

    ImGui::Spacing();

    ImGui::Text("Password:");
    ImGui::PushItemWidth(-1);
    ImGui::InputText("##Password", m_passwordBuffer, sizeof(m_passwordBuffer), ImGuiInputTextFlags_Password);
    ImGui::PopItemWidth();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Login", ImVec2(-1, 0))) {
        auto& accountManager = AccountManager::getInstance();
        if (accountManager.login(m_usernameBuffer, m_passwordBuffer)) {
            m_done = true;
            m_visible = false;
            ImGui::CloseCurrentPopup();
        } else {
            m_loginError = true;
            m_errorMessage = "Invalid username or password";
        }
    }

    ImGui::Spacing();
    ImGui::TextWrapped("Don't have an account? Register for free Community access.");
}

void LoginDialog::renderRegisterTab() {
    ImGui::Spacing();

    if (m_registerError) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::TextWrapped("%s", m_errorMessage.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    ImGui::Text("Username:");
    ImGui::PushItemWidth(-1);
    ImGui::InputText("##RegUsername", m_usernameBuffer, sizeof(m_usernameBuffer));
    ImGui::PopItemWidth();

    ImGui::Spacing();

    ImGui::Text("Email:");
    ImGui::PushItemWidth(-1);
    ImGui::InputText("##Email", m_emailBuffer, sizeof(m_emailBuffer));
    ImGui::PopItemWidth();

    ImGui::Spacing();

    ImGui::Text("Password:");
    ImGui::PushItemWidth(-1);
    ImGui::InputText("##RegPassword", m_passwordBuffer, sizeof(m_passwordBuffer), ImGuiInputTextFlags_Password);
    ImGui::PopItemWidth();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Register", ImVec2(-1, 0))) {
        auto& accountManager = AccountManager::getInstance();
        if (accountManager.registerAccount(m_usernameBuffer, m_emailBuffer, m_passwordBuffer)) {
            m_done = true;
            m_visible = false;
            ImGui::CloseCurrentPopup();
        } else {
            m_registerError = true;
            m_errorMessage = "Registration failed. Please check your input.";
        }
    }

    ImGui::Spacing();
    ImGui::TextWrapped("By registering, you agree to the Terms of Service.");
    ImGui::TextWrapped("Community accounts have access to 4K export and basic features.");
}

void LoginDialog::renderActivationTab() {
    ImGui::Spacing();

    if (m_activationError) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
        ImGui::TextWrapped("%s", m_errorMessage.c_str());
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    ImGui::TextWrapped("Enter your Aether Studio Pro activation code:");
    ImGui::Spacing();

    ImGui::Text("Activation Code:");
    ImGui::PushItemWidth(-1);
    ImGui::InputText("##ActivationCode", m_activationCodeBuffer, sizeof(m_activationCodeBuffer));
    ImGui::PopItemWidth();

    ImGui::Spacing();
    ImGui::TextWrapped("Format: XXXX-XXXX-XXXX-XXXX");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button("Activate", ImVec2(-1, 0))) {
        auto& accountManager = AccountManager::getInstance();
        if (accountManager.activateLicense(m_activationCodeBuffer)) {
            m_done = true;
            m_visible = false;
            ImGui::CloseCurrentPopup();
        } else {
            m_activationError = true;
            m_errorMessage = "Invalid activation code. Please check and try again.";
        }
    }

    ImGui::Spacing();
    ImGui::TextWrapped("Pro features include:");
    ImGui::BulletText("8K+ export");
    ImGui::BulletText("10-bit 4:2:2 / HDR support");
    ImGui::BulletText("Advanced AI tools");
    ImGui::BulletText("Cloud rendering");
}

} // namespace aether
