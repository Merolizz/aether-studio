#include "aether/NodeGraphModel.h"

namespace aether {

NodeGraphModel::NodeGraphModel() = default;

uint32_t NodeGraphModel::nextNodeId() {
    return m_nextId++;
}

uint32_t NodeGraphModel::addNode(NodeType type, float x, float y) {
    Node n;
    n.id = nextNodeId();
    n.type = type;
    n.x = x;
    n.y = y;
    n.title = nodeTypeName(type);
    int inCount = inputPortCount(type);
    int outCount = outputPortCount(type);
    for (int i = 0; i < inCount; i++)
        n.inputPortNames.push_back(std::string("in") + std::to_string(i));
    for (int i = 0; i < outCount; i++)
        n.outputPortNames.push_back(std::string("out") + std::to_string(i));
    m_nodes.push_back(std::move(n));
    return m_nodes.back().id;
}

void NodeGraphModel::removeNode(uint32_t nodeId) {
    m_nodes.erase(
        std::remove_if(m_nodes.begin(), m_nodes.end(), [nodeId](const Node& n) { return n.id == nodeId; }),
        m_nodes.end());
    m_connections.erase(
        std::remove_if(m_connections.begin(), m_connections.end(),
            [nodeId](const NodeConnection& c) { return c.source.nodeId == nodeId || c.dest.nodeId == nodeId; }),
        m_connections.end());
}

bool NodeGraphModel::addConnection(const NodePort& source, const NodePort& dest) {
    if (source.nodeId == dest.nodeId) return false;
    for (const auto& c : m_connections)
        if (c.source.nodeId == source.nodeId && c.source.portIndex == source.portIndex
            && c.dest.nodeId == dest.nodeId && c.dest.portIndex == dest.portIndex)
            return false;
    m_connections.push_back({ source, dest });
    return true;
}

void NodeGraphModel::removeConnection(size_t connectionIndex) {
    if (connectionIndex < m_connections.size())
        m_connections.erase(m_connections.begin() + static_cast<std::ptrdiff_t>(connectionIndex));
}

void NodeGraphModel::setNodePosition(uint32_t nodeId, float x, float y) {
    Node* n = nodeById(nodeId);
    if (n) { n->x = x; n->y = y; }
}

Node* NodeGraphModel::nodeById(uint32_t id) {
    for (auto& n : m_nodes)
        if (n.id == id) return &n;
    return nullptr;
}

const Node* NodeGraphModel::nodeById(uint32_t id) const {
    for (const auto& n : m_nodes)
        if (n.id == id) return &n;
    return nullptr;
}

const char* NodeGraphModel::nodeTypeName(NodeType t) {
    switch (t) {
        case NodeType::Transform: return "Transform";
        case NodeType::Blur: return "Blur";
        case NodeType::Merge: return "Merge";
        case NodeType::Mask: return "Mask";
        case NodeType::Glow: return "Glow";
    }
    return "Unknown";
}

int NodeGraphModel::inputPortCount(NodeType t) {
    switch (t) {
        case NodeType::Transform: return 1;
        case NodeType::Blur: return 1;
        case NodeType::Merge: return 2;
        case NodeType::Mask: return 2;
        case NodeType::Glow: return 1;
    }
    return 0;
}

int NodeGraphModel::outputPortCount(NodeType t) {
    return 1;
}

} // namespace aether
