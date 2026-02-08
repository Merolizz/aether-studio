#include "aether/EncoderBackend.h"

namespace aether {

class FFmpegEncoder : public IEncoderBackend {
public:
    bool open(const EncodeParams& params) override { (void)params; return true; }
    bool encodeFrame(const void* rgbData, uint32_t width, uint32_t height) override {
        (void)rgbData; (void)width; (void)height; return true;
    }
    void close() override {}
};

std::unique_ptr<IEncoderBackend> createEncoderBackend() {
    return std::make_unique<FFmpegEncoder>();
}

} // namespace aether
