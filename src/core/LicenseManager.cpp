#include "../../include/aether/LicenseManager.h"
#include "../../include/aether/HardwareID.h"
#include <iostream>
// #include <sstream>
#include <algorithm>
#include <chrono>

namespace aether {

LicenseManager &LicenseManager::getInstance() {
  static LicenseManager instance;
  return instance;
}

bool LicenseManager::initialize() {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (m_initialized.load()) {
    return true;
  }

  // Get HWID asynchronously
  std::cout << "Debug: LicenseManager getting HWID instance" << std::endl;
  auto &hardwareID = HardwareID::getInstance();
  std::cout << "Debug: LicenseManager calling getHWID" << std::endl;
  m_hwid = hardwareID.getHWID();
  std::cout << "Debug: LicenseManager getHWID returned: "
            << (m_hwid.empty() ? "empty" : "value") << std::endl;

  if (m_hwid.empty()) {
    // Generate HWID if not available
    std::cout << "Debug: LicenseManager generating HWID" << std::endl;
    m_hwid = hardwareID.generateHWID();
    std::cout << "Debug: LicenseManager generated HWID" << std::endl;
  }

  m_initialized.store(true);
  std::cout << "License Manager initialized" << std::endl;
  return true;
}

void LicenseManager::shutdown() {
  m_shouldStop.store(true);

  if (m_validationThread.joinable()) {
    m_validationThread.join();
  }

  m_initialized.store(false);
}

void LicenseManager::validateLicenseAsync(const std::string &licenseKey) {
  if (m_isChecking.load()) {
    std::cerr << "License validation already in progress" << std::endl;
    return;
  }

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_licenseKey = licenseKey;
    m_status = LicenseCheckStatus::Checking;
  }

  m_isChecking.store(true);

  // Start async validation in separate thread
  if (m_validationThread.joinable()) {
    m_validationThread.join();
  }

  m_validationThread =
      std::thread(&LicenseManager::validationWorker, this, licenseKey);
}

void LicenseManager::validationWorker(const std::string &licenseKey) {
  // Simulate async validation delay
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  if (m_shouldStop.load()) {
    return;
  }

  LicenseCheckStatus result = LicenseCheckStatus::Invalid;
  std::string message;

  // Validate license
  if (validateLicenseWithHWID(licenseKey)) {
    result = LicenseCheckStatus::Valid;
    message = "License validated successfully";

    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_licenseType = parseLicenseType(licenseKey);
    }
  } else {
    result = LicenseCheckStatus::Invalid;
    message = "License validation failed";
  }

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_status = result;
  }

  m_isChecking.store(false);

  // Call callback if set
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_validationCallback) {
      m_validationCallback(result, message);
    }
  }

  std::cout << "License validation completed: " << message << std::endl;
}

bool LicenseManager::validateLicenseWithHWID(const std::string &licenseKey) {
  if (licenseKey.empty()) {
    return false;
  }

  // Get current HWID
  std::string currentHWID = getCurrentHWID();
  if (currentHWID.empty()) {
    std::cerr << "Failed to get HWID for license validation" << std::endl;
    return false;
  }

  // Simple validation: check if license key contains HWID hash
  // In production, this would be more sophisticated (server validation,
  // encryption, etc.)

  // For now, check if license key format is valid
  // Format: AETHER-XXXX-XXXX-XXXX-XXXX (where XXXX could be HWID-based)
  if (licenseKey.length() < 20) {
    return false;
  }

  // Check prefix
  if (licenseKey.substr(0, 6) != "AETHER") {
    return false;
  }

  // Basic format validation
  size_t dashCount = std::count(licenseKey.begin(), licenseKey.end(), '-');
  if (dashCount < 4) {
    return false;
  }

  // In a real implementation, you would:
  // 1. Decrypt license key
  // 2. Extract HWID from license
  // 3. Compare with current HWID
  // 4. Check expiration date
  // 5. Validate with license server

  // For now, accept any license key starting with "AETHER" as valid
  // This is a placeholder - replace with actual validation logic
  return true;
}

LicenseType
LicenseManager::parseLicenseType(const std::string &licenseKey) const {
  // Parse license type from key
  // Format: AETHER-{TYPE}-XXXX-XXXX-XXXX

  if (licenseKey.length() < 10) {
    return LicenseType::Trial;
  }

  // Extract type from license key
  size_t firstDash = licenseKey.find('-');
  if (firstDash == std::string::npos || firstDash + 1 >= licenseKey.length()) {
    return LicenseType::Trial;
  }

  size_t secondDash = licenseKey.find('-', firstDash + 1);
  if (secondDash == std::string::npos) {
    return LicenseType::Trial;
  }

  std::string typeStr =
      licenseKey.substr(firstDash + 1, secondDash - firstDash - 1);
  std::transform(typeStr.begin(), typeStr.end(), typeStr.begin(), ::toupper);

  if (typeStr == "TRIAL") {
    return LicenseType::Trial;
  } else if (typeStr == "STD" || typeStr == "STANDARD") {
    return LicenseType::Standard;
  } else if (typeStr == "PRO" || typeStr == "PROFESSIONAL") {
    return LicenseType::Professional;
  } else if (typeStr == "ENT" || typeStr == "ENTERPRISE") {
    return LicenseType::Enterprise;
  }

  return LicenseType::Trial;
}

std::string LicenseManager::getCurrentHWID() const {
  // `getCurrentHWID()` is const, so we can't mutate `m_hwid` here.
  // If we want caching, we should either make `m_hwid` mutable or
  // provide a non-const accessor that performs caching.
  auto &hardwareID = HardwareID::getInstance();
  std::string hwid = hardwareID.getHWID();
  if (hwid.empty()) {
    hwid = hardwareID.generateHWID();
  }
  return hwid;
}

bool LicenseManager::validateHWID(const std::string &licenseKey,
                                  const std::string &expectedHWID) const {
  std::string currentHWID = getCurrentHWID();
  return currentHWID == expectedHWID;
}

} // namespace aether
