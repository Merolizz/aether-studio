#pragma once

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>

namespace aether {

enum class NodeRole {
    Master,  // Job distributor
    Worker   // Job executor
};

enum class JobStatus {
    Pending,
    Processing,
    Completed,
    Failed
};

struct RenderJob {
    std::string jobId;
    std::string filePath;
    uint32_t startFrame = 0;
    uint32_t endFrame = 0;
    uint32_t width = 1920;
    uint32_t height = 1080;
    JobStatus status = JobStatus::Pending;
    std::string resultPath;
};

struct NetworkNode {
    std::string nodeId;
    std::string ipAddress;
    uint16_t port = 0;
    bool isOnline = false;
    uint32_t availableCores = 0;
    std::string gpuName;
};

class AetherLink {
public:
    using JobCompletedCallback = std::function<void(const RenderJob& job)>;
    
    // Singleton access
    static AetherLink& getInstance();
    
    // Delete copy constructor and assignment operator
    AetherLink(const AetherLink&) = delete;
    AetherLink& operator=(const AetherLink&) = delete;

    // Initialization
    bool initialize(NodeRole role, uint16_t port = 8888);
    void shutdown();

    // Node management
    bool startServer();
    void stopServer();
    bool connectToNode(const std::string& ipAddress, uint16_t port);
    void disconnectFromNode(const std::string& nodeId);
    std::vector<NetworkNode> getConnectedNodes() const;
    
    // Job management (Master role)
    std::string submitJob(const RenderJob& job);
    bool cancelJob(const std::string& jobId);
    RenderJob getJobStatus(const std::string& jobId) const;
    std::vector<RenderJob> getAllJobs() const;
    
    // Discovery
    void discoverNodes();
    void setAutoDiscovery(bool enable) { m_autoDiscovery = enable; }
    
    // Callbacks
    void setJobCompletedCallback(JobCompletedCallback callback) { m_jobCompletedCallback = callback; }
    
    // Getters
    NodeRole getRole() const { return m_role; }
    bool isServerRunning() const { return m_serverRunning; }
    std::string getNodeId() const { return m_nodeId; }

private:
    AetherLink() = default;
    ~AetherLink() = default;

    void serverThread();
    void workerThread();
    void discoveryThread();
    std::string generateNodeId();

    NodeRole m_role = NodeRole::Master;
    uint16_t m_port = 8888;
    std::string m_nodeId;
    
    std::atomic<bool> m_serverRunning{false};
    std::atomic<bool> m_shouldStop{false};
    std::thread m_serverThread;
    std::thread m_workerThread;
    std::thread m_discoveryThread;
    
    std::vector<NetworkNode> m_connectedNodes;
    std::vector<RenderJob> m_jobQueue;
    
    JobCompletedCallback m_jobCompletedCallback;
    bool m_autoDiscovery = false;
    
    mutable std::mutex m_mutex;
    bool m_initialized = false;
};

} // namespace aether
