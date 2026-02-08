#pragma once

#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <functional>

namespace aether {

class Window {
public:
    using ResizeCallback = std::function<void(int, int)>;
    using KeyCallback = std::function<void(int, int, int, int)>;
    using MouseButtonCallback = std::function<void(int, int, int)>;
    using CursorPosCallback = std::function<void(double, double)>;

    Window(int width, int height, const std::string& title);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool initialize();
    void shutdown();
    
    bool shouldClose() const;
    void pollEvents();
    void swapBuffers();
    
    GLFWwindow* getHandle() const { return m_window; }
    
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    
    void setResizeCallback(ResizeCallback callback) { m_resizeCallback = callback; }
    void setKeyCallback(KeyCallback callback) { m_keyCallback = callback; }
    void setMouseButtonCallback(MouseButtonCallback callback) { m_mouseButtonCallback = callback; }
    void setCursorPosCallback(CursorPosCallback callback) { m_cursorPosCallback = callback; }

private:
    void loadWindowIcon(const std::string& iconPath);
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);

    GLFWwindow* m_window = nullptr;
    int m_width;
    int m_height;
    std::string m_title;
    
    ResizeCallback m_resizeCallback;
    KeyCallback m_keyCallback;
    MouseButtonCallback m_mouseButtonCallback;
    CursorPosCallback m_cursorPosCallback;
};

} // namespace aether
