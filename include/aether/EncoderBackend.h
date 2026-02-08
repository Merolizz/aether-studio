#pragma once

#include <string>
#include <cstdint>
#include <memory>

namespace aether {

struct EncodeParams {
    std::string outputPath;
    uint32_t width = 1920;
    uint32_t height = 1080;
    double fps = 24.0;
    uint32_t bitrateKbps = 0;
    std::string codec;
};

class IEncoderBackend {
public:
    virtual ~IEncoderBackend() = default;
    virtual bool open(const EncodeParams& params) = 0;
    virtual bool encodeFrame(const void* rgbData, uint32_t width, uint32_t height) = 0;
    virtual void close() = 0;
};

std::unique_ptr<IEncoderBackend> createEncoderBackend();

} // namespace aether
