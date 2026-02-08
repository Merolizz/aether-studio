#pragma once

#include <memory>
#include "Window.h"

namespace aether {

class VulkanRenderer;
#ifdef AETHER_USE_DIRECTX
#ifdef AETHER_DIRECTX_11
class DirectX11Renderer;
#else
class DirectX12Renderer;
#endif
#endif
class ImGuiManager;
class ProjectStartupDialog;
class LoginDialog;
class WorkspaceUI;
class MainMenuBar;
class StatusBar;
class PreferencesDialog;
class SplashScreen;

class Application {
public:
    Application();
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    bool initialize();
    void shutdown();
    void run();

private:
    std::unique_ptr<Window> m_window;
#ifdef AETHER_USE_DIRECTX
#ifdef AETHER_DIRECTX_11
    std::unique_ptr<DirectX11Renderer> m_renderer;
#else
    std::unique_ptr<DirectX12Renderer> m_renderer;
#endif
#else
    std::unique_ptr<VulkanRenderer> m_renderer;
#endif
    std::unique_ptr<ImGuiManager> m_imguiManager;
    std::unique_ptr<ProjectStartupDialog> m_startupDialog;
    std::unique_ptr<LoginDialog> m_loginDialog;
    std::unique_ptr<WorkspaceUI> m_workspaceUI;
    std::unique_ptr<MainMenuBar> m_mainMenuBar;
    std::unique_ptr<StatusBar> m_statusBar;
    std::unique_ptr<PreferencesDialog> m_preferencesDialog;
    std::unique_ptr<SplashScreen> m_splashScreen;
    
    bool m_initialized = false;
    bool m_shouldClose = false;
    bool m_showLoginDialog = false;
    bool m_projectLoaded = false;
    bool m_showPreferences = false;
    bool m_splashScreenShown = false;
};

} // namespace aether
