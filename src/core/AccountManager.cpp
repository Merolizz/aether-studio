#include "../../include/aether/Account.h"
#include "../../include/aether/HardwareID.h"
#include <nlohmann/json.hpp>
#ifdef _WIN32
#include <shlobj.h>
#include <windows.h>

#endif
#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>


namespace aether {

AccountManager &AccountManager::getInstance() {
  static AccountManager instance;
  return instance;
}

bool AccountManager::login(const std::string &username,
                           const std::string &password) {
  std::lock_guard<std::mutex> lock(m_mutex);

  // TODO: Implement actual authentication with server
  // For now, just set logged in state
  if (username.empty() || password.empty()) {
    return false;
  }

  m_accountInfo.username = username;
  m_accountInfo.isLoggedIn = true;
  m_accountInfo.type = AccountType::Community; // Default to community

  // Check if there's a saved license
  checkLicenseStatus();

  std::cout << "Logged in as: " << username << std::endl;
  return true;
}

void AccountManager::logout() {
  std::lock_guard<std::mutex> lock(m_mutex);

  m_accountInfo = AccountInfo{};
  std::cout << "Logged out" << std::endl;
}

bool AccountManager::registerAccount(const std::string &username,
                                     const std::string &email,
                                     const std::string &password) {
  std::lock_guard<std::mutex> lock(m_mutex);

  // TODO: Implement actual registration with server
  if (username.empty() || email.empty() || password.empty()) {
    return false;
  }

  // Basic validation
  if (username.length() < 3) {
    std::cerr << "Username must be at least 3 characters" << std::endl;
    return false;
  }

  if (email.find('@') == std::string::npos) {
    std::cerr << "Invalid email address" << std::endl;
    return false;
  }

  m_accountInfo.username = username;
  m_accountInfo.email = email;
  m_accountInfo.isLoggedIn = true;
  m_accountInfo.type = AccountType::Community;
  m_accountInfo.licenseStatus = LicenseStatus::None;

  std::cout << "Account registered: " << username << std::endl;
  return true;
}

bool AccountManager::activateLicense(const std::string &activationCode) {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (activationCode.empty()) {
    return false;
  }

  // Validate activation code format
  if (!validateActivationCode(activationCode)) {
    std::cerr << "Invalid activation code format" << std::endl;
    m_accountInfo.licenseStatus = LicenseStatus::Invalid;
    return false;
  }

  // Validate HWID lock
  if (!validateHWID(activationCode)) {
    std::cerr << "Activation code does not match this hardware" << std::endl;
    m_accountInfo.licenseStatus = LicenseStatus::Invalid;
    return false;
  }

  m_accountInfo.activationCode = activationCode;
  m_accountInfo.type = AccountType::Studio;
  m_accountInfo.licenseStatus = LicenseStatus::Valid;

  // Lock to current HWID
  auto &hwid = HardwareID::getInstance();
  m_lockedHWID = hwid.getHWID();
  m_hwidLocked = true;

  // Save license info
  saveLicenseInfo();

  std::cout << "License activated successfully and locked to HWID" << std::endl;
  return true;
}

bool AccountManager::validateLicense() {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_accountInfo.activationCode.empty()) {
    m_accountInfo.licenseStatus = LicenseStatus::None;
    return false;
  }

  // Re-validate activation code format
  if (!validateActivationCode(m_accountInfo.activationCode)) {
    m_accountInfo.licenseStatus = LicenseStatus::Invalid;
    return false;
  }

  // Check HWID lock
  if (m_hwidLocked) {
    auto &hwid = HardwareID::getInstance();
    if (!hwid.matchesCurrentHWID(m_lockedHWID)) {
      std::cerr << "HWID mismatch - license locked to different hardware"
                << std::endl;
      m_accountInfo.licenseStatus = LicenseStatus::Invalid;
      return false;
    }
  }

  m_accountInfo.licenseStatus = LicenseStatus::Valid;
  return true;
}

void AccountManager::checkLicenseStatus() {
  std::lock_guard<std::mutex> lock(m_mutex);

  // Try to load saved license info
  if (m_accountInfo.activationCode.empty()) {
    loadLicenseInfo();
  }

  if (m_accountInfo.activationCode.empty()) {
    m_accountInfo.licenseStatus = LicenseStatus::None;
    return;
  }

  validateLicense();
}

bool AccountManager::validateActivationCode(const std::string &code) {
  // Basic validation - check format
  // Format: XXXX-XXXX-XXXX-XXXX (16 characters, 4 groups of 4)
  if (code.length() != 19) { // 16 chars + 3 dashes
    return false;
  }

  // Check format: XXXX-XXXX-XXXX-XXXX
  for (size_t i = 0; i < code.length(); i++) {
    if (i == 4 || i == 9 || i == 14) {
      if (code[i] != '-') {
        return false;
      }
    } else {
      if (!std::isalnum(static_cast<unsigned char>(code[i]))) {
        return false;
      }
    }
  }

  // TODO: Implement RSA-256 signature verification
  // For now, just check basic format
  // In production, this would verify the signature against a public key

  return true;
}

bool AccountManager::validateHWID(const std::string &activationCode) {
  (void)activationCode; // Reserved for future server-side validation
  // Get current HWID
  auto &hwid = HardwareID::getInstance();
  std::string currentHWID = hwid.generateHWID();

  if (currentHWID.empty()) {
    std::cerr << "Failed to generate HWID" << std::endl;
    return false;
  }

  // If license is already locked, check if HWID matches
  if (m_hwidLocked && !m_lockedHWID.empty()) {
    return hwid.matchesCurrentHWID(m_lockedHWID);
  }

  // For new activation, lock to current HWID
  // In production, activation code would be validated against server
  // and server would verify HWID matches
  m_lockedHWID = currentHWID;
  m_hwidLocked = true;

  return true;
}

void AccountManager::saveLicenseInfo() {
  // TODO: Save to registry or config file
  // For now, just log
  std::cout << "License info saved (HWID locked: " << m_lockedHWID << ")"
            << std::endl;
}

bool AccountManager::loadLicenseInfo() {
  // TODO: Load from registry or config file
  // For now, return false (no saved license)
  return false;
}

std::string AccountManager::hashActivationCode(const std::string &code) {
  // Simple hash for demonstration using std::hash
  // In production, use proper cryptographic hash (SHA-256) with OpenSSL or
  // similar
  std::hash<std::string> hasher;
  size_t hashValue = hasher(code);

  std::stringstream ss;
  ss << std::hex << hashValue;
  return ss.str();
}

} // namespace aether
