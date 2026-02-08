#pragma once

namespace aether {

class MainMenuBar {
public:
    MainMenuBar();
    ~MainMenuBar() = default;

    MainMenuBar(const MainMenuBar&) = delete;
    MainMenuBar& operator=(const MainMenuBar&) = delete;

    void render();
    bool shouldShowPreferences() const { return m_showPreferences; }
    void resetPreferencesFlag() { m_showPreferences = false; }

private:
    void renderFileMenu();
    void renderEditMenu();
    void renderViewMenu();
    void renderWorkspaceMenu();
    void renderHelpMenu();
    
    bool m_showAbout = false;
    bool m_showPreferences = false;
};

} // namespace aether
