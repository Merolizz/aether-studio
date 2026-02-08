#include "Window.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <filesystem>

namespace aether {

Window::Window(int width, int height, const std::string& title)
    : m_width(width), m_height(height), m_title(title) {
}

Window::~Window() {
    shutdown();
}

bool Window::initialize() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, cursorPosCallback);

    // Load and set window icon
    loadWindowIcon("assets/icon.png");

    return true;
}

void Window::shutdown() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

bool Window::shouldClose() const {
    return m_window ? glfwWindowShouldClose(m_window) : true;
}

void Window::pollEvents() {
    glfwPollEvents();
}

void Window::swapBuffers() {
    // Not used with Vulkan, but kept for compatibility
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win) {
        win->m_width = width;
        win->m_height = height;
        if (win->m_resizeCallback) {
            win->m_resizeCallback(width, height);
        }
    }
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->m_keyCallback) {
        win->m_keyCallback(key, scancode, action, mods);
    }
}

void Window::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->m_mouseButtonCallback) {
        win->m_mouseButtonCallback(button, action, mods);
    }
}

void Window::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
    if (win && win->m_cursorPosCallback) {
        win->m_cursorPosCallback(xpos, ypos);
    }
}

void Window::loadWindowIcon(const std::string& iconPath) {
    // Check if icon file exists
    if (!std::filesystem::exists(iconPath)) {
        std::cerr << "Window icon not found: " << iconPath << " (using default icon)" << std::endl;
        return;
    }
    
    // In a full implementation, this would:
    // 1. Load PNG using stb_image or similar library
    // 2. Create GLFWimage structure
    // 3. Call glfwSetWindowIcon()
    
    // For now, this is a placeholder
    // Actual implementation would require:
    // - stb_image.h for PNG loading
    // - GLFWimage structure population
    // - glfwSetWindowIcon(m_window, 1, &image)
    
    // Placeholder: Just log that icon loading would happen here
    std::cout << "Window icon loading placeholder: " << iconPath << std::endl;
    std::cout << "Note: Actual icon loading requires stb_image or similar library" << std::endl;
    
    // TODO: Implement actual icon loading when image loading library is available
    // Example code (when stb_image is available):
    /*
    int width, height, channels;
    unsigned char* data = stbi_load(iconPath.c_str(), &width, &height, &channels, 4);
    if (data) {
        GLFWimage image;
        image.width = width;
        image.height = height;
        image.pixels = data;
        glfwSetWindowIcon(m_window, 1, &image);
        stbi_image_free(data);
        std::cout << "Window icon loaded: " << iconPath << std::endl;
    } else {
        std::cerr << "Failed to load window icon: " << iconPath << std::endl;
    }
    */
}

} // namespace aether
