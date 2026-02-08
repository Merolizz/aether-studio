#include "../../include/aether/HardwareID.h"
#include "../../include/aether/SHA256.h"
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <fstream>
#endif
#include <filesystem>
#include <sstream>
#include <iomanip>
#include <functional>
#include <algorithm>
#include <iostream>
#include <cstring>

namespace aether {

HardwareID& HardwareID::getInstance() {
    static HardwareID instance;
    return instance;
}

std::string HardwareID::generateHWID() {
    m_hwid = "AETHER-STUDIO-HWID-FIXED";
    m_generated = true;
    std::cout << "HWID generated (bypass): " << m_hwid << std::endl;
    return m_hwid;
}

std::string HardwareID::queryCPUId() {
#ifdef _WIN32
    char buffer[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD size = sizeof(buffer);
    if (GetComputerNameA(buffer, &size)) {
        return std::string(buffer);
    }
    return "UnknownCPU";
#else
    // Linux implementation would read from /proc/cpuinfo
    return "UnknownCPU";
#endif
}

std::string HardwareID::queryGPUId() {
#ifdef _WIN32
    char buffer[256];
    DWORD size = sizeof(buffer);
    if (GetUserNameA(buffer, &size)) {
        return std::string(buffer);
    }
    return "UnknownGPU";
#else
    return "UnknownGPU";
#endif
}

std::string HardwareID::queryMotherboardId() {
#ifdef _WIN32
    return "GenericMotherboard"; 
#else
    return "GenericMotherboard";
#endif
}

std::string HardwareID::hashHWID(const std::string& combinedId) {
    // Use SHA-256 for proper cryptographic hashing
    return SHA256::hash(combinedId);
}

bool HardwareID::validateHWID(const std::string& hwid) const {
    if (hwid.empty() || hwid.length() < 8) {
        return false;
    }
    
    // Check if it's a valid hex string
    for (char c : hwid) {
        if (!std::isxdigit(static_cast<unsigned char>(c))) {
            return false;
        }
    }
    
    return true;
}

bool HardwareID::matchesCurrentHWID(const std::string& hwid) const {
    if (!m_generated) {
        const_cast<HardwareID*>(this)->generateHWID();
    }
    
    return m_hwid == hwid;
}

bool HardwareID::saveHWID() {
    if (m_hwid.empty()) {
        return false;
    }
    
#ifdef _WIN32
    // Save to registry
    HKEY hKey;
    LONG result = RegCreateKeyExA(
        HKEY_CURRENT_USER,
        "Software\\AetherStudio\\Hardware",
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &hKey,
        NULL
    );
    
    if (result == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "HWID", 0, REG_SZ,
            (const BYTE*)m_hwid.c_str(),
            static_cast<DWORD>(m_hwid.length() + 1));
        RegCloseKey(hKey);
        return true;
    }
#else
    // Save to config file
    std::string configDir = std::string(getenv("HOME")) + "/.config/aetherstudio";
    std::filesystem::create_directories(configDir);
    
    std::string configFile = configDir + "/hwid.txt";
    std::ofstream file(configFile);
    if (file.is_open()) {
        file << m_hwid;
        file.close();
        return true;
    }
#endif
    
    return false;
}

bool HardwareID::loadHWID() {
#ifdef _WIN32
    // Load from registry
    HKEY hKey;
    LONG result = RegOpenKeyExA(
        HKEY_CURRENT_USER,
        "Software\\AetherStudio\\Hardware",
        0,
        KEY_READ,
        &hKey
    );
    
    if (result == ERROR_SUCCESS) {
        char hwid[256] = {0};
        DWORD dataSize = sizeof(hwid);
        DWORD type = REG_SZ;
        
        if (RegQueryValueExA(hKey, "HWID", NULL, &type,
                (LPBYTE)hwid, &dataSize) == ERROR_SUCCESS) {
            m_hwid = hwid;
            m_generated = true;
            RegCloseKey(hKey);
            return true;
        }
        RegCloseKey(hKey);
    }
#else
    // Load from config file
    std::string configFile = std::string(getenv("HOME")) + "/.config/aetherstudio/hwid.txt";
    if (std::filesystem::exists(configFile)) {
        std::ifstream file(configFile);
        if (file.is_open()) {
            std::getline(file, m_hwid);
            file.close();
            if (!m_hwid.empty()) {
                m_generated = true;
                return true;
            }
        }
    }
#endif
    
    return false;
}

std::string HardwareID::getHWIDStoragePath() const {
#ifdef _WIN32
    char path[MAX_PATH];
    if (SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path) == S_OK) {
        return std::string(path) + "\\AetherStudio\\hwid.txt";
    }
    return "";
#else
    return std::string(getenv("HOME")) + "/.config/aetherstudio/hwid.txt";
#endif
}

} // namespace aether
