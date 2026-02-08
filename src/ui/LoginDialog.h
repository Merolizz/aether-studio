#pragma once

#include <string>

namespace aether {

class LoginDialog {
public:
    LoginDialog();
    ~LoginDialog() = default;

    LoginDialog(const LoginDialog&) = delete;
    LoginDialog& operator=(const LoginDialog&) = delete;

    void show();
    void close();
    void render();

    bool isVisible() const { return m_visible; }
    bool isDone() const { return m_done; }
    bool shouldShowActivation() const { return m_showActivation; }

private:
    void renderLoginTab();
    void renderRegisterTab();
    void renderActivationTab();

    bool m_visible = false;
    bool m_done = false;
    bool m_showActivation = false;
    int m_selectedTab = 0; // 0 = Login, 1 = Register, 2 = Activation

    // Login fields
    char m_usernameBuffer[256] = "";
    char m_passwordBuffer[256] = "";
    char m_emailBuffer[256] = "";
    char m_activationCodeBuffer[64] = "";
    
    bool m_loginError = false;
    bool m_registerError = false;
    bool m_activationError = false;
    std::string m_errorMessage;
};

} // namespace aether
