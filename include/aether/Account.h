#pragma once

#include <string>
#include <memory>
#include <mutex>

namespace aether {

enum class AccountType {
    Community,  // Free tier
    Studio      // Pro tier
};

enum class LicenseStatus {
    None,           // No license
    Trial,          // Trial period
    Valid,          // Valid license
    Expired,        // License expired
    Invalid         // Invalid license
};

struct AccountInfo {
    std::string username;
    std::string email;
    AccountType type = AccountType::Community;
    LicenseStatus licenseStatus = LicenseStatus::None;
    std::string activationCode;
    bool isLoggedIn = false;
};

class AccountManager {
public:
    // Singleton access
    static AccountManager& getInstance();
    
    // Delete copy constructor and assignment operator
    AccountManager(const AccountManager&) = delete;
    AccountManager& operator=(const AccountManager&) = delete;

    // Account management
    bool login(const std::string& username, const std::string& password);
    void logout();
    bool registerAccount(const std::string& username, const std::string& email, const std::string& password);
    
    // License management
    bool activateLicense(const std::string& activationCode);
    bool validateLicense();
    void checkLicenseStatus();
    bool isHWIDLocked() const { return m_hwidLocked; }
    std::string getLockedHWID() const { return m_lockedHWID; }
    
    // Getters
    bool isLoggedIn() const { return m_accountInfo.isLoggedIn; }
    const AccountInfo& getAccountInfo() const { return m_accountInfo; }
    AccountType getAccountType() const { return m_accountInfo.type; }
    LicenseStatus getLicenseStatus() const { return m_accountInfo.licenseStatus; }
    bool hasProLicense() const { 
        return m_accountInfo.isLoggedIn && 
               m_accountInfo.type == AccountType::Studio && 
               m_accountInfo.licenseStatus == LicenseStatus::Valid; 
    }
    
    // Feature checks
    bool canExport4K() const { return hasProLicense() || m_accountInfo.type == AccountType::Community; }
    bool canExport8K() const { return hasProLicense(); }
    bool canUse10Bit() const { return hasProLicense(); }
    bool canUseHDR() const { return hasProLicense(); }
    bool canUseAdvancedAI() const { return hasProLicense(); }
    bool canUseCloudRendering() const { return hasProLicense(); }

private:
    AccountManager() = default;
    ~AccountManager() = default;

    AccountInfo m_accountInfo;
    mutable std::mutex m_mutex;
    
    // HWID locking
    bool m_hwidLocked = false;
    std::string m_lockedHWID;
    
    // License validation
    bool validateActivationCode(const std::string& code);
    bool validateHWID(const std::string& activationCode);
    std::string hashActivationCode(const std::string& code);
    void saveLicenseInfo();
    bool loadLicenseInfo();
};

} // namespace aether
