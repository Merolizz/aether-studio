#pragma once

#include <string>
#include <vector>

namespace aether {

class HardwareID {
public:
    // Singleton access
    static HardwareID& getInstance();
    
    // Delete copy constructor and assignment operator
    HardwareID(const HardwareID&) = delete;
    HardwareID& operator=(const HardwareID&) = delete;

    // HWID generation
    std::string generateHWID();
    std::string getHWID() const { return m_hwid; }
    bool isValid() const { return !m_hwid.empty(); }
    
    // HWID components
    std::string getCPUId() const { return m_cpuId; }
    std::string getGPUId() const { return m_gpuId; }
    std::string getMotherboardId() const { return m_motherboardId; }
    
    // HWID validation
    bool validateHWID(const std::string& hwid) const;
    bool matchesCurrentHWID(const std::string& hwid) const;
    
    // Persistent storage
    bool saveHWID();
    bool loadHWID();

private:
    HardwareID() = default;
    ~HardwareID() = default;

    std::string queryCPUId();
    std::string queryGPUId();
    std::string queryMotherboardId();
    std::string hashHWID(const std::string& combinedId);
    std::string getHWIDStoragePath() const;

    std::string m_hwid;
    std::string m_cpuId;
    std::string m_gpuId;
    std::string m_motherboardId;
    bool m_generated = false;
};

} // namespace aether
