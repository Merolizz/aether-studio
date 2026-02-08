#pragma once

#include <string>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>
#include <future>

namespace aether {

// NOTE: `LicenseStatus` already exists in `Account.h`.
// This enum is specific to the async license *check* flow.
enum class LicenseCheckStatus {
    Unknown,        // Not checked yet
    Valid,          // License is valid
    Invalid,        // License is invalid
    Expired,        // License has expired
    Checking,       // Currently checking license
    Error           // Error during license check
};

enum class LicenseType {
    Trial,          // Trial license
    Standard,       // Standard license
    Professional,   // Professional license
    Enterprise      // Enterprise license
};

class LicenseManager {
public:
    using LicenseValidationCallback = std::function<void(LicenseCheckStatus status, const std::string& message)>;
    
    // Singleton access
    static LicenseManager& getInstance();
    
    // Delete copy constructor and assignment operator
    LicenseManager(const LicenseManager&) = delete;
    LicenseManager& operator=(const LicenseManager&) = delete;
    
    // Initialization
    bool initialize();
    void shutdown();
    
    // Async license validation
    void validateLicenseAsync(const std::string& licenseKey);
    bool isLicenseValid() const { return m_status == LicenseCheckStatus::Valid; }
    LicenseCheckStatus getStatus() const { return m_status; }
    LicenseType getLicenseType() const { return m_licenseType; }
    
    // Callbacks
    void setValidationCallback(LicenseValidationCallback callback) { 
        std::lock_guard<std::mutex> lock(m_mutex);
        m_validationCallback = callback; 
    }
    
    // HWID management
    std::string getCurrentHWID() const;
    bool validateHWID(const std::string& licenseKey, const std::string& expectedHWID) const;
    
    // License info
    std::string getLicenseKey() const { 
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_licenseKey; 
    }
    
    bool isChecking() const { return m_isChecking.load(); }

private:
    LicenseManager() = default;
    ~LicenseManager() = default;
    
    // Async validation worker
    void validationWorker(const std::string& licenseKey);
    
    // HWID-based validation
    bool validateLicenseWithHWID(const std::string& licenseKey);
    
    // License key parsing
    LicenseType parseLicenseType(const std::string& licenseKey) const;
    
    mutable std::mutex m_mutex;
    std::atomic<bool> m_initialized{false};
    std::atomic<bool> m_isChecking{false};
    std::atomic<LicenseCheckStatus> m_status{LicenseCheckStatus::Unknown};
    
    LicenseType m_licenseType = LicenseType::Trial;
    std::string m_licenseKey;
    std::string m_hwid;
    
    LicenseValidationCallback m_validationCallback;
    std::thread m_validationThread;
    std::atomic<bool> m_shouldStop{false};
};

} // namespace aether
