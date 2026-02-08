#pragma once

namespace aether {

class PreferencesDialog {
public:
    PreferencesDialog();
    ~PreferencesDialog() = default;

    PreferencesDialog(const PreferencesDialog&) = delete;
    PreferencesDialog& operator=(const PreferencesDialog&) = delete;

    void show();
    void close();
    void render();

    bool isVisible() const { return m_visible; }

private:
    void renderGeneralTab();
    void renderDisplayTab();
    void renderPerformanceTab();
    void renderKeymapTab();

    bool m_visible = false;
    int m_selectedTab = 0;
};

} // namespace aether
