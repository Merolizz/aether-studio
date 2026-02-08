#include "aether/KeyframeModel.h"
#include <algorithm>

namespace aether {

KeyframeModel::KeyframeModel() = default;

void KeyframeModel::setNodeId(uint32_t nodeId) {
    m_nodeId = nodeId;
}

void KeyframeModel::setParameters(const std::vector<ParameterKeyframes>& params) {
    m_parameters = params;
}

void KeyframeModel::addKeyframe(const std::string& parameterId, int64_t timeMs, double value) {
    for (auto& p : m_parameters) {
        if (p.parameterId != parameterId) continue;
        Keyframe kf;
        kf.timeMs = timeMs;
        kf.value = value;
        p.keyframes.push_back(kf);
        std::sort(p.keyframes.begin(), p.keyframes.end(), [](const Keyframe& a, const Keyframe& b) { return a.timeMs < b.timeMs; });
        return;
    }
}

void KeyframeModel::removeKeyframe(const std::string& parameterId, size_t keyframeIndex) {
    for (auto& p : m_parameters) {
        if (p.parameterId != parameterId) continue;
        if (keyframeIndex < p.keyframes.size())
            p.keyframes.erase(p.keyframes.begin() + static_cast<std::ptrdiff_t>(keyframeIndex));
        return;
    }
}

void KeyframeModel::setKeyframeValue(const std::string& parameterId, size_t keyframeIndex, double value) {
    for (auto& p : m_parameters) {
        if (p.parameterId != parameterId) continue;
        if (keyframeIndex < p.keyframes.size())
            p.keyframes[keyframeIndex].value = value;
        return;
    }
}

void KeyframeModel::setKeyframeTime(const std::string& parameterId, size_t keyframeIndex, int64_t timeMs) {
    for (auto& p : m_parameters) {
        if (p.parameterId != parameterId) continue;
        if (keyframeIndex < p.keyframes.size()) {
            p.keyframes[keyframeIndex].timeMs = timeMs;
            std::sort(p.keyframes.begin(), p.keyframes.end(), [](const Keyframe& a, const Keyframe& b) { return a.timeMs < b.timeMs; });
        }
        return;
    }
}

double KeyframeModel::evaluate(const std::string& parameterId, int64_t timeMs) const {
    for (const auto& p : m_parameters) {
        if (p.parameterId != parameterId) continue;
        if (p.keyframes.empty()) return 0.0;
        if (timeMs <= p.keyframes.front().timeMs) return p.keyframes.front().value;
        if (timeMs >= p.keyframes.back().timeMs) return p.keyframes.back().value;
        for (size_t i = 0; i + 1 < p.keyframes.size(); i++) {
            int64_t t0 = p.keyframes[i].timeMs;
            int64_t t1 = p.keyframes[i + 1].timeMs;
            if (timeMs >= t0 && timeMs <= t1) {
                double v0 = p.keyframes[i].value;
                double v1 = p.keyframes[i + 1].value;
                double u = static_cast<double>(timeMs - t0) / static_cast<double>(t1 - t0);
                return v0 + u * (v1 - v0);
            }
        }
        return p.keyframes.back().value;
    }
    return 0.0;
}

} // namespace aether
