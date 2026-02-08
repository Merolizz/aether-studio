// Aether Studio – Qt6 entry point (modernized: dynamic Vulkan init, flexible FFmpeg path, safe exit)
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#undef VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#include <QApplication>
#include <QMessageBox>
#include <QString>
#include <QDir>

#include "qt/MainWindow.h"
#include "aether/LicenseManager.h"

#include <optional>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#endif

namespace {

std::string getSettingsPath() {
#ifdef _WIN32
    char exePath[MAX_PATH];
    if (GetModuleFileNameA(nullptr, exePath, MAX_PATH)) {
        std::string s(exePath);
        std::string::size_type last = s.find_last_of("/\\");
        if (last != std::string::npos)
            return s.substr(0, last + 1) + "settings.ini";
    }
#endif
    return "settings.ini";
}

std::string readIniValue(const std::string& filePath, const std::string& key) {
    std::ifstream f(filePath);
    if (!f.good()) return {};
    std::string line;
    while (std::getline(f, line)) {
        std::string::size_type eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string k = line.substr(0, eq);
        while (!k.empty() && (k.back() == ' ' || k.back() == '\t')) k.pop_back();
        while (!k.empty() && (k.front() == ' ' || k.front() == '\t')) k.erase(0, 1);
        if (k != key) continue;
        std::string v = line.substr(eq + 1);
        while (!v.empty() && (v.front() == ' ' || v.front() == '\t')) v.erase(0, 1);
        while (!v.empty() && (v.back() == ' ' || v.back() == '\t' || v.back() == '\r')) v.pop_back();
        return v;
    }
    return {};
}

// Dynamically enumerate instance extensions (no fixed array – avoids stack corruption).
// Returns error message on failure; on success, *outNames is filled with extension names to enable.
std::optional<std::string> getInstanceExtensionNames(std::vector<std::string>* outNames) {
    if (!outNames) return std::string("Internal error: outNames is null");
    outNames->clear();

    uint32_t extensionCount = 0;
    VkResult res = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    if (res != VK_SUCCESS) {
        return std::string("Vulkan vkEnumerateInstanceExtensionProperties (count) failed: ") + std::to_string(static_cast<int>(res));
    }
    if (extensionCount == 0) {
        return std::nullopt; // no extensions, still valid
    }

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    uint32_t count = extensionCount;
    res = vkEnumerateInstanceExtensionProperties(nullptr, &count, availableExtensions.data());
    if (res != VK_SUCCESS) {
        return std::string("Vulkan vkEnumerateInstanceExtensionProperties (list) failed: ") + std::to_string(static_cast<int>(res));
    }
    if (count > extensionCount) count = extensionCount;

    auto hasExtension = [&availableExtensions, count](const char* name) {
        for (uint32_t i = 0; i < count; i++) {
            if (strcmp(availableExtensions[i].extensionName, name) == 0) return true;
        }
        return false;
    };

#ifdef _WIN32
    const char* required[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
#else
    const char* required[] = { VK_KHR_SURFACE_EXTENSION_NAME };
#endif
    for (const char* name : required) {
        if (hasExtension(name))
            outNames->push_back(name);
        else
            return std::string("Required Vulkan instance extension not found: ") + name;
    }
    return std::nullopt;
}

#ifdef _WIN32
// Check that critical Vulkan DLL can be loaded. Returns error message or nullopt on success.
std::optional<std::string> checkVulkanDll() {
    std::string prefix;
    char exePath[MAX_PATH];
    if (GetModuleFileNameA(nullptr, exePath, MAX_PATH)) {
        std::string s(exePath);
        std::string::size_type last = s.find_last_of("/\\");
        if (last != std::string::npos)
            prefix = s.substr(0, last + 1);
    }
    const char* dllName = "vulkan-1.dll";
    std::string path1 = prefix + dllName;
    HMODULE h = LoadLibraryA(path1.c_str());
    if (!h)
        h = LoadLibraryA(dllName);
    if (!h) {
        DWORD err = GetLastError();
        return std::string("Critical DLL not found: vulkan-1.dll. Please install Vulkan Runtime or Vulkan SDK (e.g. 1.4.335). Error code: ") + std::to_string(err);
    }
    FreeLibrary(h);
    return std::nullopt;
}
#endif

// PreFlightCheck: Vulkan (dynamic init) and FFmpeg (flexible path). Returns error message or nullopt on success.
std::optional<std::string> PreFlightCheck() {
#ifdef _WIN32
    // 0) Critical DLL: vulkan-1.dll must be loadable
    auto dllErr = checkVulkanDll();
    if (dllErr) return dllErr;
#endif

    // 1) Vulkan: dynamic enumeration + create/destroy instance
    {
        std::vector<std::string> extensionNames;
        auto err = getInstanceExtensionNames(&extensionNames);
        if (err) return err;

        std::vector<const char*> ppNames(extensionNames.size());
        for (size_t i = 0; i < extensionNames.size(); i++)
            ppNames[i] = extensionNames[i].c_str();

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "AetherStudioQt";
        appInfo.applicationVersion = 1;
        appInfo.pEngineName = "Aether";
        appInfo.engineVersion = 1;
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(ppNames.size());
        createInfo.ppEnabledExtensionNames = ppNames.empty() ? nullptr : ppNames.data();
        createInfo.enabledLayerCount = 0;

        VkInstance instance = VK_NULL_HANDLE;
        VkResult res = vkCreateInstance(&createInfo, nullptr, &instance);
        if (res != VK_SUCCESS) {
            return std::string("Vulkan vkCreateInstance failed (") + std::to_string(static_cast<int>(res)) + ").";
        }
        vkDestroyInstance(instance, nullptr);
    }

    // 2) FFmpeg DLLs: search order = application dir, then settings FFmpegDir/bin, then C:/ffmpeg/bin
#ifdef _WIN32
    {
        std::string settingsPath = getSettingsPath();
        std::string ffmpegRoot = readIniValue(settingsPath, "FFmpegDir");
        if (ffmpegRoot.empty()) ffmpegRoot = "C:/ffmpeg";
        bool skipCheck = (readIniValue(settingsPath, "SkipFFmpegCheck") == "1");
        std::ifstream f(settingsPath);
        if (!f.good())
            skipCheck = true;
        if (skipCheck)
            ; // skip
        else {
            auto dirHasAvcodec = [](const std::string& dir) -> bool {
                if (dir.empty()) return false;
                std::string path = dir;
                if (path.back() != '/' && path.back() != '\\') path += '\\';
                path += "avcodec*.dll";
                WIN32_FIND_DATAA fd;
                HANDLE h = FindFirstFileA(path.c_str(), &fd);
                bool found = (h != INVALID_HANDLE_VALUE);
                if (h != INVALID_HANDLE_VALUE) FindClose(h);
                return found;
            };
            auto toBinDir = [](std::string root) {
                while (!root.empty() && (root.back() == '/' || root.back() == '\\')) root.pop_back();
                std::string lower = root;
                for (auto& c : lower) if (c >= 'A' && c <= 'Z') c += 32;
                if (lower.size() >= 4 && (lower.compare(lower.size() - 4, 4, "\\bin") == 0 || lower.compare(lower.size() - 4, 4, "/bin") == 0))
                    return root;
                return root + "\\bin";
            };

            QString appDir = QCoreApplication::applicationDirPath();
            std::string appDirStr = appDir.isEmpty() ? "" : QDir::toNativeSeparators(appDir).toStdString();
            std::string appDirFfmpegBin = appDirStr.empty() ? "" : (appDirStr + (appDirStr.back() == '\\' ? "" : "\\") + "ffmpeg\\bin");
            std::string searchDirs[] = { appDirStr, appDirFfmpegBin, toBinDir(ffmpegRoot), "C:\\ffmpeg\\bin" };
            bool found = false;
            for (const auto& d : searchDirs) {
                if (dirHasAvcodec(d)) { found = true; break; }
            }
            if (!found) {
                return std::string("FFmpeg DLLs not found. Searched: (1) app dir, (2) app dir/ffmpeg/bin, (3) ") + toBinDir(ffmpegRoot) +
                    ", (4) C:\\ffmpeg\\bin. Set File -> Settings -> FFmpeg folder, or add SkipFFmpegCheck=1 in settings.ini.";
            }
        }
    }
#endif

    // 3) Assets
    {
        auto exists = [](const std::string& path) -> bool {
            std::ifstream f(path);
            return f.good();
        };
        std::string prefix;
#ifdef _WIN32
        char exePath[MAX_PATH];
        if (GetModuleFileNameA(nullptr, exePath, MAX_PATH)) {
            std::string s(exePath);
            std::string::size_type last = s.find_last_of("/\\");
            if (last != std::string::npos)
                prefix = s.substr(0, last + 1);
        }
#endif
        const char* assets[] = { "assets/icon.png", "assets/splash.png" };
        for (const char* rel : assets) {
            bool ok = exists(rel);
            if (!ok && !prefix.empty()) ok = exists(prefix + rel);
            if (!ok) {
                return std::string("Required asset not found: ") + rel + ". Run from the application directory.";
            }
        }
    }

    return std::nullopt;
}

} // namespace

int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

#ifdef _WIN32
    // Add application directory to DLL search path so FFmpeg (and other) DLLs next to exe are found first
    QString appDir = QCoreApplication::applicationDirPath();
    if (!appDir.isEmpty()) {
        QByteArray path = appDir.toLocal8Bit();
        if (SetDllDirectoryA(path.constData()) == 0) {
            QDir::setCurrent(appDir);
        }
    }
#endif

    std::optional<std::string> err = PreFlightCheck();
    if (err) {
        QMessageBox::critical(nullptr, QObject::tr("Aether Studio - Startup Error"), QString::fromStdString(*err));
        return 1;
    }

    aether::LicenseManager::getInstance().initialize();
    std::string hwid = aether::LicenseManager::getInstance().getCurrentHWID();
    if (hwid.empty()) {
        QMessageBox::warning(nullptr, QObject::tr("Aether Studio - HWID"),
            QObject::tr("Hardware ID could not be retrieved. Licensing features may be limited. You can continue anyway."));
    }

    aether::MainWindow w;
    w.show();
    return a.exec();
}
