#pragma once

namespace aether {

class StatusBar {
public:
    StatusBar();
    ~StatusBar() = default;

    StatusBar(const StatusBar&) = delete;
    StatusBar& operator=(const StatusBar&) = delete;

    void render();

private:
    void renderProjectInfo();
    void renderSystemInfo();
    void renderMemoryInfo();
    void renderRenderMode();
};

} // namespace aether
