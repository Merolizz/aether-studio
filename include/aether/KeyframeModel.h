#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace aether {

enum class KeyframeInterpolation {
    Linear,
    Hold,
    Bezier
};

struct Keyframe {
    int64_t timeMs = 0;
    double value = 0.0;
    KeyframeInterpolation interpolation = KeyframeInterpolation::Linear;
};

struct ParameterKeyframes {
    std::string parameterId;
    std::string displayName;
    double minValue = 0.0;
    double maxValue = 1.0;
    std::vector<Keyframe> keyframes;
};

class KeyframeModel {
public:
    KeyframeModel();
    void setNodeId(uint32_t nodeId);
    uint32_t nodeId() const { return m_nodeId; }
    void setParameters(const std::vector<ParameterKeyframes>& params);
    const std::vector<ParameterKeyframes>& parameters() const { return m_parameters; }
    void addKeyframe(const std::string& parameterId, int64_t timeMs, double value);
    void removeKeyframe(const std::string& parameterId, size_t keyframeIndex);
    void setKeyframeValue(const std::string& parameterId, size_t keyframeIndex, double value);
    void setKeyframeTime(const std::string& parameterId, size_t keyframeIndex, int64_t timeMs);
    double evaluate(const std::string& parameterId, int64_t timeMs) const;

private:
    uint32_t m_nodeId = 0;
    std::vector<ParameterKeyframes> m_parameters;
};

} // namespace aether
