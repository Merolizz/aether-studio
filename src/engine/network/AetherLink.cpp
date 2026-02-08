#include "../../include/aether/AetherLink.h"
#include <iostream>
#include <sstream>
#include <random>
#include <algorithm>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

namespace aether {

AetherLink& AetherLink::getInstance() {
    static AetherLink instance;
    return instance;
}

bool AetherLink::initialize(NodeRole role, uint16_t port) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_initialized) {
        return true;
    }
    
    m_role = role;
    m_port = port;
    m_nodeId = generateNodeId();
    
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Failed to initialize Winsock" << std::endl;
        return false;
    }
#endif
    
    m_initialized = true;
    std::cout << "Aether Link initialized as " << (role == NodeRole::Master ? "Master" : "Worker") 
              << " on port " << port << std::endl;
    return true;
}

void AetherLink::shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        return;
    }
    
    stopServer();
    
    if (m_serverThread.joinable()) {
        m_shouldStop = true;
        m_serverThread.join();
    }
    
    if (m_workerThread.joinable()) {
        m_workerThread.join();
    }
    
    if (m_discoveryThread.joinable()) {
        m_discoveryThread.join();
    }
    
#ifdef _WIN32
    WSACleanup();
#endif
    
    m_initialized = false;
}

std::string AetherLink::generateNodeId() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << "NODE-";
    for (int i = 0; i < 8; i++) {
        ss << std::hex << dis(gen);
    }
    
    return ss.str();
}

bool AetherLink::startServer() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_serverRunning) {
        return true;
    }
    
    m_shouldStop = false;
    m_serverRunning = true;
    
    if (m_role == NodeRole::Master) {
        m_serverThread = std::thread(&AetherLink::serverThread, this);
    }
    
    m_workerThread = std::thread(&AetherLink::workerThread, this);
    
    if (m_autoDiscovery) {
        m_discoveryThread = std::thread(&AetherLink::discoveryThread, this);
    }
    
    std::cout << "Aether Link server started" << std::endl;
    return true;
}

void AetherLink::stopServer() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_serverRunning) {
        return;
    }
    
    m_shouldStop = true;
    m_serverRunning = false;
    
    std::cout << "Aether Link server stopped" << std::endl;
}

void AetherLink::serverThread() {
    std::cout << "Aether Link server thread started on port " << m_port << std::endl;
    
#ifdef _WIN32
    SOCKET listenSocket = INVALID_SOCKET;
    SOCKET clientSocket = INVALID_SOCKET;
    struct sockaddr_in serverAddr, clientAddr;
    int clientAddrLen = sizeof(clientAddr);
    
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        return;
    }
    
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(m_port);
    
    if (bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        return;
    }
    
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        return;
    }
    
    std::cout << "Server listening on port " << m_port << std::endl;
    
    while (!m_shouldStop) {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(listenSocket, &readSet);
        
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int result = select(0, &readSet, NULL, NULL, &timeout);
        if (result > 0 && FD_ISSET(listenSocket, &readSet)) {
            clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
            if (clientSocket != INVALID_SOCKET) {
                char clientIP[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
                std::cout << "Worker connected: " << clientIP << std::endl;
                
                NetworkNode node;
                node.nodeId = "NODE-" + std::string(clientIP);
                node.ipAddress = clientIP;
                node.port = ntohs(clientAddr.sin_port);
                node.isOnline = true;
                
                {
                    std::lock_guard<std::mutex> lock(m_mutex);
                    m_connectedNodes.push_back(node);
                }
                
                closesocket(clientSocket);
            }
        }
    }
    
    closesocket(listenSocket);
#else
    std::cout << "Server thread (Linux not fully implemented)" << std::endl;
    while (!m_shouldStop) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
#endif
}

void AetherLink::workerThread() {
    if (m_role != NodeRole::Worker) {
        return;
    }
    
    std::cout << "Worker thread started" << std::endl;
    
    while (!m_shouldStop) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (auto& job : m_jobQueue) {
                if (job.status == JobStatus::Pending) {
                    job.status = JobStatus::Processing;
                    std::cout << "Processing job: " << job.jobId << std::endl;
                    
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    
                    job.status = JobStatus::Completed;
                    
                    if (m_jobCompletedCallback) {
                        m_jobCompletedCallback(job);
                    }
                }
            }
        }
    }
}

void AetherLink::discoveryThread() {
    std::cout << "Discovery thread started (placeholder)" << std::endl;
    
    while (!m_shouldStop) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

bool AetherLink::connectToNode(const std::string& ipAddress, uint16_t port) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (const auto& node : m_connectedNodes) {
        if (node.ipAddress == ipAddress && node.port == port) {
            return true;
        }
    }
    
    NetworkNode node;
    node.nodeId = "NODE-" + ipAddress;
    node.ipAddress = ipAddress;
    node.port = port;
    node.isOnline = true;
    
    m_connectedNodes.push_back(node);
    
    std::cout << "Connected to node: " << ipAddress << ":" << port << std::endl;
    return true;
}

void AetherLink::disconnectFromNode(const std::string& nodeId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_connectedNodes.erase(
        std::remove_if(m_connectedNodes.begin(), m_connectedNodes.end(),
            [&nodeId](const NetworkNode& node) { return node.nodeId == nodeId; }),
        m_connectedNodes.end()
    );
}

std::vector<NetworkNode> AetherLink::getConnectedNodes() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_connectedNodes;
}

std::string AetherLink::submitJob(const RenderJob& job) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (m_role != NodeRole::Master) {
        std::cerr << "Only master can submit jobs" << std::endl;
        return "";
    }
    
    RenderJob newJob = job;
    if (newJob.jobId.empty()) {
        newJob.jobId = "JOB-" + std::to_string(m_jobQueue.size() + 1);
    }
    
    newJob.status = JobStatus::Pending;
    m_jobQueue.push_back(newJob);
    
    std::cout << "Job submitted: " << newJob.jobId << std::endl;
    return newJob.jobId;
}

bool AetherLink::cancelJob(const std::string& jobId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (auto& job : m_jobQueue) {
        if (job.jobId == jobId) {
            if (job.status == JobStatus::Pending) {
                job.status = JobStatus::Failed;
                return true;
            }
        }
    }
    
    return false;
}

RenderJob AetherLink::getJobStatus(const std::string& jobId) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (const auto& job : m_jobQueue) {
        if (job.jobId == jobId) {
            return job;
        }
    }
    
    return RenderJob{};
}

std::vector<RenderJob> AetherLink::getAllJobs() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_jobQueue;
}

void AetherLink::discoverNodes() {
    std::cout << "Discovering nodes on network..." << std::endl;
}

} // namespace aether
