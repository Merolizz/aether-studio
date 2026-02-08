#pragma once

#include <vector>
#include <mutex>
#include <cstdint>
#include <cstddef>

namespace aether {

struct DecodedFrame {
    std::vector<uint8_t> rgb;
    int width = 0;
    int height = 0;
    int64_t timestampMs = 0;
};

class FrameRingBuffer {
public:
    static constexpr size_t kDefaultSlots = 8;

    explicit FrameRingBuffer(size_t slotCount = kDefaultSlots);

    void push(int width, int height, const uint8_t* rgbData, size_t rgbSize, int64_t timestampMs);
    bool getLatest(DecodedFrame& out) const;
    void clear();

private:
    size_t m_slotCount;
    std::vector<DecodedFrame> m_slots;
    size_t m_writeIndex = 0;
    size_t m_readIndex = 0;
    mutable std::mutex m_mutex;
};

} // namespace aether
