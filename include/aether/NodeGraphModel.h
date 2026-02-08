#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace aether {

enum class NodeType {
    Transform,
    Blur,
    Merge,
    Mask,
    Glow
};

struct NodePort {
    uint32_t nodeId = 0;
    uint32_t portIndex = 0;
};

struct NodeConnection {
    NodePort source;
    NodePort dest;
};

struct Node {
    uint32_t id = 0;
    NodeType type = NodeType::Transform;
    float x = 0.f;
    float y = 0.f;
    std::string title;
    std::vector<std::string> inputPortNames;
    std::vector<std::string> outputPortNames;
};

class NodeGraphModel {
public:
    NodeGraphModel();
    uint32_t addNode(NodeType type, float x, float y);
    void removeNode(uint32_t nodeId);
    bool addConnection(const NodePort& source, const NodePort& dest);
    void removeConnection(size_t connectionIndex);
    void setNodePosition(uint32_t nodeId, float x, float y);

    const std::vector<Node>& nodes() const { return m_nodes; }
    const std::vector<NodeConnection>& connections() const { return m_connections; }
    Node* nodeById(uint32_t id);
    const Node* nodeById(uint32_t id) const;

    static const char* nodeTypeName(NodeType t);
    static int inputPortCount(NodeType t);
    static int outputPortCount(NodeType t);

private:
    uint32_t nextNodeId();
    std::vector<Node> m_nodes;
    std::vector<NodeConnection> m_connections;
    uint32_t m_nextId = 1;
};

} // namespace aether
