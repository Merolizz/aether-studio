#include "aether/FrameRingBuffer.h"
#include <algorithm>
#include <cstring>

namespace aether {

FrameRingBuffer::FrameRingBuffer(size_t slotCount)
    : m_slotCount(slotCount > 0 ? slotCount : kDefaultSlots)
    , m_slots(m_slotCount)
{
}

void FrameRingBuffer::push(int width, int height, const uint8_t* rgbData, size_t rgbSize, int64_t timestampMs) {
    std::lock_guard<std::mutex> lock(m_mutex);
    DecodedFrame& slot = m_slots[m_writeIndex % m_slotCount];
    slot.width = width;
    slot.height = height;
    slot.timestampMs = timestampMs;
    slot.rgb.resize(rgbSize);
    if (rgbData && rgbSize)
        std::memcpy(slot.rgb.data(), rgbData, rgbSize);
    m_writeIndex++;
}

bool FrameRingBuffer::getLatest(DecodedFrame& out) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_writeIndex == 0) return false;
    const DecodedFrame& slot = m_slots[(m_writeIndex - 1) % m_slotCount];
    out.width = slot.width;
    out.height = slot.height;
    out.timestampMs = slot.timestampMs;
    out.rgb = slot.rgb;
    return true;
}

void FrameRingBuffer::clear() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_writeIndex = 0;
    m_readIndex = 0;
}

} // namespace aether
