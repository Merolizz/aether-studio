#include "../../include/aether/ProxyManager.h"
#include "../../include/aether/HardwareOrchestrator.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace aether {

ProxyManager& ProxyManager::getInstance() {
    static ProxyManager instance;
    return instance;
}

bool ProxyManager::initialize() {
    std::cout << "Debug: ProxyManager::initialize start" << std::endl;
    if (m_initialized) {
        return true;
    }
    
    // Create proxy directory
    /*
    try {
        std::filesystem::create_directories(m_proxyDirectory);
    } catch (const std::exception& e) {
        std::cerr << "Failed to create proxy directory: " << e.what() << std::endl;
        return false;
    }
    */
    
    std::cout << "Debug: ProxyManager directory creation bypassed" << std::endl;
    // Check for Tensor Core support
    // m_hasTensorCoreSupport = checkTensorCoreSupport();
    m_hasTensorCoreSupport = false; // Bypass check
    
    m_initialized = true;
    std::cout << "Proxy Manager initialized (Bypassed)" << std::endl;
    if (m_hasTensorCoreSupport) {
        std::cout << "Tensor Core support detected - AI Upscale available" << std::endl;
    }
    
    return true;
}

void ProxyManager::shutdown() {
    m_initialized = false;
}

bool ProxyManager::createProxy(const std::string& sourcePath, const ProxySettings& settings) {
    if (!m_initialized) {
        return false;
    }
    
    if (!std::filesystem::exists(sourcePath)) {
        std::cerr << "Source file does not exist: " << sourcePath << std::endl;
        return false;
    }
    
    // Check if proxy already exists
    std::string proxyPath = getProxyPath(sourcePath, settings);
    if (std::filesystem::exists(proxyPath)) {
        std::cout << "Proxy already exists: " << proxyPath << std::endl;
        return true;
    }
    
    // Create proxy
    return encodeProxy(sourcePath, settings);
}

bool ProxyManager::isProxyAvailable(const std::string& sourcePath, const ProxySettings& settings) const {
    std::string proxyPath = getProxyPath(sourcePath, settings);
    return std::filesystem::exists(proxyPath);
}

std::string ProxyManager::getProxyPath(const std::string& sourcePath, const ProxySettings& settings) const {
    if (!settings.outputPath.empty()) {
        return settings.outputPath;
    }
    
    return generateProxyPath(sourcePath, settings);
}

bool ProxyManager::canUseAIUpscale() const {
    return m_hasTensorCoreSupport;
}

bool ProxyManager::upscaleWithAI(const std::string& sourcePath, uint32_t targetWidth, uint32_t targetHeight, const std::string& outputPath) {
    if (!canUseAIUpscale()) {
        std::cerr << "AI Upscale not available - Tensor Core support required" << std::endl;
        return false;
    }
    
    if (!std::filesystem::exists(sourcePath)) {
        std::cerr << "Source file does not exist: " << sourcePath << std::endl;
        return false;
    }
    
    // Placeholder for AI upscale implementation
    std::cout << "AI Upscale placeholder: " << sourcePath << " -> " << outputPath 
              << " (" << targetWidth << "x" << targetHeight << ")" << std::endl;
    
    // For now, just create a placeholder file
    std::ofstream file(outputPath);
    if (file.is_open()) {
        file << "AI Upscaled video placeholder\n";
        file << "Source: " << sourcePath << "\n";
        file << "Target: " << targetWidth << "x" << targetHeight << "\n";
        file.close();
        return true;
    }
    
    return false;
}

bool ProxyManager::checkTensorCoreSupport() const {
    auto& hardwareOrchestrator = HardwareOrchestrator::getInstance();
    const auto& gpu = hardwareOrchestrator.getPrimaryGPU();
    
    // Check if GPU is NVIDIA RTX series (has Tensor Cores)
    std::string gpuName = gpu.name;
    std::transform(gpuName.begin(), gpuName.end(), gpuName.begin(), ::tolower);
    
    // RTX series GPUs have Tensor Cores
    if (gpuName.find("rtx") != std::string::npos) {
        return true;
    }
    
    return false;
}

std::string ProxyManager::generateProxyPath(const std::string& sourcePath, const ProxySettings& settings) const {
    std::filesystem::path source(sourcePath);
    std::string stem = source.stem().string();
    std::string extension = source.extension().string();
    
    std::stringstream ss;
    ss << stem << "_proxy_";
    
    switch (settings.quality) {
        case ProxyQuality::Low: ss << "low"; break;
        case ProxyQuality::Medium: ss << "med"; break;
        case ProxyQuality::High: ss << "high"; break;
    }
    
    if (settings.targetWidth > 0 && settings.targetHeight > 0) {
        ss << "_" << settings.targetWidth << "x" << settings.targetHeight;
    }
    
    ss << extension;
    
    return (std::filesystem::path(m_proxyDirectory) / ss.str()).string();
}

bool ProxyManager::encodeProxy(const std::string& sourcePath, const ProxySettings& settings) {
    std::string proxyPath = generateProxyPath(sourcePath, settings);
    
    std::cout << "Creating proxy: " << sourcePath << " -> " << proxyPath << std::endl;
    
    // FFmpeg command construction (would execute via system or FFmpeg library)
    std::string commandFile = proxyPath + ".cmd";
    std::ofstream cmdFile(commandFile);
    
    if (cmdFile.is_open()) {
        // Calculate target resolution based on quality
        uint32_t targetW = settings.targetWidth;
        uint32_t targetH = settings.targetHeight;
        
        if (targetW == 0 || targetH == 0) {
            targetW = 960;
            targetH = 540;
        }
        
        // Build FFmpeg command
        std::string ffmpegCmd = "ffmpeg -i \"" + sourcePath + "\"";
        ffmpegCmd += " -vf scale=" + std::to_string(targetW) + ":" + std::to_string(targetH);
        ffmpegCmd += " -c:v " + settings.codec;
        if (settings.bitrate > 0) {
            ffmpegCmd += " -b:v " + std::to_string(settings.bitrate) + "k";
        }
        ffmpegCmd += " -c:a copy";
        ffmpegCmd += " \"" + proxyPath + "\"";
        
        cmdFile << ffmpegCmd;
        cmdFile.close();
        
        std::cout << "FFmpeg command saved to: " << commandFile << std::endl;
        
        // Create placeholder file
        std::ofstream file(proxyPath + ".pending");
        if (file.is_open()) {
            file << "Proxy generation pending\n";
            file << "Source: " << sourcePath << "\n";
            file << "Command: " << ffmpegCmd << "\n";
            file.close();
        }
        
        return true;
    }
    
    return false;
}

} // namespace aether
