#include "Application.h"
#include "../../include/aether/Account.h" // AccountManager is in Account.h
#include "../../include/aether/BackgroundCache.h"
#include "../../include/aether/Clipboard.h"
#include "../../include/aether/GlobalProjectContext.h"
#include "../../include/aether/KeymapManager.h"
#include "../../include/aether/LicenseManager.h"
#include "../../include/aether/MemoryManager.h"
#include "../../include/aether/MemoryWatchdog.h"
#include "../../include/aether/ProxyManager.h"
#include "../../include/aether/ResourceStreamer.h"
#include "../../include/aether/SplashScreen.h"
#include "../../include/aether/Tool.h"
#include "../../include/aether/UndoRedo.h"
#include "../../include/aether/WorkspaceManager.h"
#ifdef AETHER_USE_DIRECTX
#ifdef AETHER_DIRECTX_11
#include "../engine/render/DirectX11Renderer.h"
#else
#include "../engine/render/DirectX12Renderer.h"
#endif
#else
#include "../engine/render/VulkanRenderer.h"
#endif
#include "../ui/ImGuiManager.h"
#include "../ui/LoginDialog.h"
#include "../ui/MainMenuBar.h"
#include "../ui/PreferencesDialog.h"
#include "../ui/ProjectStartupDialog.h"
#include "../ui/StatusBar.h"
#include "../ui/WorkspaceUI.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>
#include <thread>

namespace aether {

Application::Application() {}

Application::~Application() { shutdown(); }

bool Application::initialize() {
  // Create window
  m_window = std::make_unique<Window>(1280, 720, "Aether Studio");
  if (!m_window->initialize()) {
    std::cerr << "Failed to initialize window" << std::endl;
    return false;
  }

  // Initialize renderer (Vulkan or DirectX)
#ifdef AETHER_USE_DIRECTX
#ifdef AETHER_DIRECTX_11
  m_renderer = std::make_unique<DirectX11Renderer>();
#else
  m_renderer = std::make_unique<DirectX12Renderer>();
#endif
  if (!m_renderer->initialize(m_window.get())) {
    std::cerr << "Failed to initialize DirectX renderer" << std::endl;
    return false;
  }
#else
  m_renderer = std::make_unique<VulkanRenderer>();
  if (!m_renderer->initialize(m_window.get())) {
    std::cerr << "Failed to initialize Vulkan renderer" << std::endl;
    return false;
  }
#endif

  // Initialize ImGui
  m_imguiManager = std::make_unique<ImGuiManager>();
#ifdef AETHER_USE_DIRECTX
  if (!m_imguiManager->initialize(m_window.get(), m_renderer.get())) {
#else
  if (!m_imguiManager->initialize(m_window.get(), m_renderer.get())) {
#endif
    std::cerr << "Failed to initialize ImGui" << std::endl;
    return false;
  }

  // Create startup dialog
  m_startupDialog = std::make_unique<ProjectStartupDialog>();

  // Create login dialog
  m_loginDialog = std::make_unique<LoginDialog>();

  // Initialize Workspace Manager
  auto &workspaceManager = WorkspaceManager::getInstance();
  if (!workspaceManager.initialize()) {
    std::cerr << "Failed to initialize Workspace Manager" << std::endl;
    return false;
  }

  // Create Workspace UI
  m_workspaceUI = std::make_unique<WorkspaceUI>();

  // Create Main Menu Bar
  m_mainMenuBar = std::make_unique<MainMenuBar>();

  // Create Status Bar
  m_statusBar = std::make_unique<StatusBar>();

  // Create Preferences Dialog
  m_preferencesDialog = std::make_unique<PreferencesDialog>();

  // Create Splash Screen (Vulkan only; DirectX path skips splash)
#ifndef AETHER_USE_DIRECTX
  m_splashScreen = std::make_unique<SplashScreen>();
  if (!m_splashScreen->initialize(m_renderer.get(), m_window.get())) {
    std::cerr << "Warning: Failed to initialize splash screen" << std::endl;
  } else {
    m_splashScreen->show();
    m_splashScreenShown = true;
  }
#else
  m_splashScreen = std::make_unique<SplashScreen>();
  m_splashScreenShown = false;
  if (!m_projectLoaded) {
    m_startupDialog->show();
  }
#endif

  // Initialize Memory Manager
  auto &memoryManager = MemoryManager::getInstance();
  if (!memoryManager.initialize()) {
    std::cerr << "Failed to initialize Memory Manager" << std::endl;
    return false;
  }
  memoryManager.setRAMLimitPercentage(0.75f); // 75% limit

  // Initialize Memory Watchdog
  auto &memoryWatchdog = MemoryWatchdog::getInstance();
  if (!memoryWatchdog.initialize()) {
    std::cerr << "Failed to initialize Memory Watchdog" << std::endl;
    return false;
  }
  memoryWatchdog.setWarningThreshold(0.75f); // 75% warning threshold
  memoryWatchdog.startMonitoring();
  std::cout << "Debug: Memory Watchdog started" << std::endl;

  // Initialize License Manager
  auto &licenseManager = LicenseManager::getInstance();
  if (!licenseManager.initialize()) {
    std::cerr << "Failed to initialize License Manager" << std::endl;
    return false;
  }
  std::cout << "Debug: License Manager initialized" << std::endl;

  // Initialize Resource Streamer
  auto &resourceStreamer = ResourceStreamer::getInstance();
  if (!resourceStreamer.initialize()) {
    std::cerr << "Failed to initialize Resource Streamer" << std::endl;
    return false;
  }
  std::cout << "Debug: Resource Streamer initialized" << std::endl;

  // Initialize Background Cache
  auto &backgroundCache = BackgroundCache::getInstance();
  std::string cacheDir = "./cache/effects";
  if (!backgroundCache.initialize(cacheDir)) {
    std::cerr << "Failed to initialize Background Cache" << std::endl;
    return false;
  }
  std::cout << "Debug: Background Cache initialized" << std::endl;

  // Initialize Proxy Manager
  auto &proxyManager = ProxyManager::getInstance();
  if (!proxyManager.initialize()) {
    std::cerr << "Failed to initialize Proxy Manager" << std::endl;
    return false;
  }
  std::cout << "Debug: Proxy Manager initialized" << std::endl;

  // Initialize Tool Registry
  auto &toolRegistry = ToolRegistry::getInstance();
  toolRegistry.initializeDefaultTools();
  std::cout << "Debug: Tool Registry initialized" << std::endl;

  // Initialize Keymap Manager
  auto &keymapManager = KeymapManager::getInstance();
  if (!keymapManager.initialize()) {
    std::cerr << "Failed to initialize Keymap Manager" << std::endl;
    return false;
  }
  std::cout << "Debug: Keymap Manager initialized" << std::endl;

  // Setup key callback for keymap handling
  m_window->setKeyCallback([this](int key, int scancode, int action, int mods) {
    (void)scancode; // Unused platform-specific parameter
    auto &keymapManager = KeymapManager::getInstance();
    auto &workspaceManager = WorkspaceManager::getInstance();

    // Convert GLFW modifiers to KeyModifier
    KeyModifier modifiers = KeyModifier::None;
    if (mods & GLFW_MOD_CONTROL)
      modifiers = modifiers | KeyModifier::Ctrl;
    if (mods & GLFW_MOD_SHIFT)
      modifiers = modifiers | KeyModifier::Shift;
    if (mods & GLFW_MOD_ALT)
      modifiers = modifiers | KeyModifier::Alt;
    if (mods & GLFW_MOD_SUPER)
      modifiers = modifiers | KeyModifier::Super;

    // Get current workspace
    WorkspaceType currentWorkspaceType = workspaceManager.getCurrentWorkspace();
    WorkspaceContext currentWorkspace = WorkspaceContext::Global;
    switch (currentWorkspaceType) {
    case WorkspaceType::Edit:
      currentWorkspace = WorkspaceContext::Edit;
      break;
    case WorkspaceType::Animation:
      currentWorkspace = WorkspaceContext::Animation;
      break;
    case WorkspaceType::Photo:
      currentWorkspace = WorkspaceContext::Photo;
      break;
    case WorkspaceType::Color:
      currentWorkspace = WorkspaceContext::Color;
      break;
    case WorkspaceType::Audio:
      currentWorkspace = WorkspaceContext::Audio;
      break;
    default:
      currentWorkspace = WorkspaceContext::Global;
      break;
    }

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
      keymapManager.handleKeyPress(key, modifiers, currentWorkspace);
    } else if (action == GLFW_RELEASE) {
      keymapManager.handleKeyRelease(key, modifiers, currentWorkspace);
    }
  });

  m_initialized = true;
  return true;
}

void Application::shutdown() {
  // Close project if open
  auto &context = GlobalProjectContext::getInstance();
  if (context.isProjectOpen()) {
    context.closeProject();
  }

  // Logout if logged in
  auto &accountManager = AccountManager::getInstance();
  if (accountManager.isLoggedIn()) {
    accountManager.logout();
  }

  // Shutdown Keymap Manager
  auto &keymapManager = KeymapManager::getInstance();
  keymapManager.shutdown();

  // Shutdown Workspace Manager
  auto &workspaceManager = WorkspaceManager::getInstance();
  workspaceManager.shutdown();

  // Shutdown Proxy Manager
  auto &proxyManager = ProxyManager::getInstance();
  proxyManager.shutdown();

  // Shutdown Background Cache
  auto &backgroundCache = BackgroundCache::getInstance();
  backgroundCache.shutdown();

  // Shutdown Resource Streamer
  auto &resourceStreamer = ResourceStreamer::getInstance();
  resourceStreamer.shutdown();

  // Shutdown Memory Watchdog
  auto &memoryWatchdog = MemoryWatchdog::getInstance();
  memoryWatchdog.shutdown();

  // Shutdown Memory Manager
  auto &memoryManager = MemoryManager::getInstance();
  memoryManager.shutdown();

  // Shutdown License Manager
  auto &licenseManager = LicenseManager::getInstance();
  licenseManager.shutdown();

  if (m_splashScreen) {
    m_splashScreen->shutdown();
    m_splashScreen.reset();
  }

  if (m_imguiManager) {
    m_imguiManager->shutdown();
    m_imguiManager.reset();
  }

  if (m_renderer) {
    m_renderer->shutdown();
    m_renderer.reset();
  }

  if (m_window) {
    m_window->shutdown();
    m_window.reset();
  }

  m_initialized = false;
}

void Application::run() {
  if (!m_initialized) {
    std::cerr << "Application not initialized" << std::endl;
    return;
  }

  // Frame pacing variables for low CPU usage
  constexpr double TARGET_FPS = 60.0;
  constexpr double FRAME_TIME_MS = 1000.0 / TARGET_FPS;
  auto lastFrameTime = std::chrono::high_resolution_clock::now();
  auto lastUpdateTime = std::chrono::high_resolution_clock::now();

  // Check if window is minimized (for additional optimization)
  bool wasMinimized = false;

  // Main loop - optimized for low CPU usage
  while (!m_window->shouldClose() && !m_shouldClose) {
    // Check if window is minimized
    int windowWidth, windowHeight;
    glfwGetWindowSize(m_window->getHandle(), &windowWidth, &windowHeight);
    bool isMinimized = (windowWidth == 0 || windowHeight == 0);

    if (isMinimized && !wasMinimized) {
      wasMinimized = true;
      // When minimized, reduce update frequency significantly
    } else if (!isMinimized && wasMinimized) {
      wasMinimized = false;
      // Reset timing when window is restored
      lastFrameTime = std::chrono::high_resolution_clock::now();
    }

    // If minimized, sleep longer to reduce CPU usage
    if (isMinimized) {
      std::this_thread::sleep_for(
          std::chrono::milliseconds(100)); // Sleep 100ms when minimized
      m_window->pollEvents(); // Still poll events but less frequently
      continue;
    }

    // Frame pacing: Calculate time since last frame
    auto currentTime = std::chrono::high_resolution_clock::now();
    auto frameElapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                            currentTime - lastFrameTime)
                            .count() /
                        1000.0; // Convert to milliseconds

    // If frame completed too quickly, sleep to maintain target FPS
    if (frameElapsed < FRAME_TIME_MS) {
      auto sleepTime = static_cast<int>(FRAME_TIME_MS - frameElapsed);
      if (sleepTime > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
      }
    }

    lastFrameTime = std::chrono::high_resolution_clock::now();

    // Poll events (non-blocking)
    m_window->pollEvents();

    // Begin frame
    if (!m_renderer->beginFrame()) {
      // If frame acquisition failed, small sleep to prevent busy waiting
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }

    // Render ImGui
    m_imguiManager->beginFrame();

    // Show splash screen first (for 3 seconds)
    if (m_splashScreenShown && m_splashScreen && m_splashScreen->isVisible()) {
      m_splashScreen->render();

      if (m_splashScreen->shouldClose()) {
        m_splashScreen->hide();
        m_splashScreenShown = false;
        // Show startup dialog after splash
        if (!m_projectLoaded) {
          m_startupDialog->show();
        }
      }

      m_imguiManager->endFrame();
      m_renderer->beginRenderPass();
#ifdef AETHER_USE_DIRECTX
      m_imguiManager->render();
#else
      VkCommandBuffer cmdBuffer = m_renderer->getCurrentCommandBuffer();
      m_imguiManager->render(cmdBuffer);
#endif
      m_renderer->endRenderPass();
      m_renderer->endFrame();
      continue;
    }

    // Render main menu bar
    if (m_mainMenuBar) {
      m_mainMenuBar->render();

      // Check if preferences should be shown
      if (m_mainMenuBar->shouldShowPreferences()) {
        m_showPreferences = true;
        m_mainMenuBar->resetPreferencesFlag();
      }
    }

    // Show preferences dialog
    if (m_showPreferences && m_preferencesDialog) {
      m_preferencesDialog->show();
      m_preferencesDialog->render();
      if (!m_preferencesDialog->isVisible()) {
        m_showPreferences = false;
      }
    }

    // Show login dialog if needed (optional - can be shown from menu)
    if (m_showLoginDialog) {
      m_loginDialog->render();
      if (m_loginDialog->isDone()) {
        m_showLoginDialog = false;
        m_loginDialog->close();
      }
    }

    // Show startup dialog if needed (after splash screen)
    if (!m_projectLoaded && !m_splashScreenShown) {
      m_startupDialog->render();

      // Check if dialog is done
      if (m_startupDialog->isDone()) {
        auto result = m_startupDialog->getResult();
        if (result == DialogResult::Cancel) {
          m_shouldClose = true;
        } else if (result == DialogResult::NewProject) {
          // Create new project in global context
          auto &context = GlobalProjectContext::getInstance();
          auto settings = m_startupDialog->getSettings();
          if (context.createNewProject(settings)) {
            std::cout << "Project created: " << settings.projectName
                      << std::endl;
            m_startupDialog->close();
            m_projectLoaded = true;
          } else {
            std::cerr << "Failed to create project" << std::endl;
            // Keep dialog open on error
            m_startupDialog->show();
          }
        } else if (result == DialogResult::OpenProject) {
          // Load project from file
          auto &context = GlobalProjectContext::getInstance();
          auto settings = m_startupDialog->getSettings();
          if (!settings.projectPath.empty()) {
            if (context.loadProject(settings.projectPath)) {
              std::cout << "Project loaded: " << settings.projectPath
                        << std::endl;
              m_startupDialog->close();
              m_projectLoaded = true;
            } else {
              std::cerr << "Failed to load project" << std::endl;
              m_startupDialog->show();
            }
          } else {
            // No file selected, keep dialog open
            m_startupDialog->show();
          }
        }
      }
    } else {
      // Project loaded, show workspace UI
      m_workspaceUI->render();

      // Render status bar
      if (m_statusBar) {
        m_statusBar->render();
      }

      // Update memory watchdog (which wraps MemoryManager)
      auto &memoryWatchdog = MemoryWatchdog::getInstance();
      memoryWatchdog.update();

      // MemoryWatchdog will automatically trigger cleanup if over threshold

      // Update workspace
      auto &workspaceManager = WorkspaceManager::getInstance();
      static float lastWorkspaceTime = 0.0f;
      float workspaceCurrentTime = static_cast<float>(glfwGetTime());
      float deltaTime = workspaceCurrentTime - lastWorkspaceTime;
      lastWorkspaceTime = workspaceCurrentTime;
      workspaceManager.update(deltaTime);

      // Update resource streamer
      auto &resourceStreamer = ResourceStreamer::getInstance();
      resourceStreamer.updateWorkspaceResources(
          workspaceManager.getCurrentWorkspace());
    }

    m_imguiManager->endFrame();

    // Begin render pass
    m_renderer->beginRenderPass();

    // Render ImGui (Vulkan: to command buffer; DirectX: to current context/command list)
#ifdef AETHER_USE_DIRECTX
    m_imguiManager->render();
#else
    VkCommandBuffer cmdBuffer = m_renderer->getCurrentCommandBuffer();
    m_imguiManager->render(cmdBuffer);
#endif

    // End render pass
    m_renderer->endRenderPass();

    // End frame (submits command buffer / presents)
    m_renderer->endFrame();

    // Update timing for next frame
    lastUpdateTime = std::chrono::high_resolution_clock::now();
  }
}

} // namespace aether
