#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cstdint>
#include <functional>

namespace aether {

enum class PixelFormat {
    RGB8,
    RGBA8,
    RGB10,
    RGBA10,
    YUV420P,
    YUV422P,
    YUV444P,
    YUV420P10LE,
    YUV422P10LE,
    YUV444P10LE
};

enum class ColorSpace {
    BT709,
    BT2020,
    Rec2020,
    P3,
    SRGB
};

struct VideoFrame {
    std::vector<uint8_t> data;
    uint32_t width = 0;
    uint32_t height = 0;
    PixelFormat format = PixelFormat::RGB8;
    ColorSpace colorSpace = ColorSpace::BT709;
    int64_t timestamp = 0; // in microseconds
    int64_t frameNumber = 0;
};

struct VideoMetadata {
    std::string codec;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t fps = 0;
    int64_t duration = 0; // in microseconds
    uint64_t bitrate = 0;
    PixelFormat pixelFormat = PixelFormat::RGB8;
    ColorSpace colorSpace = ColorSpace::BT709;
    bool hasAudio = false;
    uint32_t audioSampleRate = 0;
    uint32_t audioChannels = 0;
};

class VideoLoader {
public:
    VideoLoader();
    ~VideoLoader();

    VideoLoader(const VideoLoader&) = delete;
    VideoLoader& operator=(const VideoLoader&) = delete;

    // File operations
    bool open(const std::string& filePath);
    void close();
    bool isOpen() const { return m_isOpen; }
    
    // Metadata
    const VideoMetadata& getMetadata() const { return m_metadata; }
    bool hasMetadata() const { return m_hasMetadata; }
    
    // Frame reading
    bool readFrame(VideoFrame& frame);
    bool seekToFrame(int64_t frameNumber);
    bool seekToTime(int64_t timestamp); // microseconds
    bool seekToPosition(double position); // 0.0 to 1.0
    
    // Current position
    int64_t getCurrentFrame() const { return m_currentFrame; }
    int64_t getCurrentTimestamp() const { return m_currentTimestamp; }
    double getCurrentPosition() const; // 0.0 to 1.0
    
    // Hardware acceleration
    bool enableHardwareAcceleration(bool enable = true);
    bool isHardwareAccelerated() const { return m_useHardwareAcceleration; }
    
    // Format support
    static bool isFormatSupported(const std::string& filePath);
    static std::vector<std::string> getSupportedFormats();

private:
    bool initializeDecoder();
    void cleanupDecoder();
    bool decodeFrame(VideoFrame& frame);
    PixelFormat convertPixelFormat(int avFormat);
    ColorSpace detectColorSpace();

    void* m_formatContext = nullptr; // AVFormatContext*
    void* m_codecContext = nullptr;  // AVCodecContext*
    void* m_frame = nullptr;         // AVFrame*
    void* m_packet = nullptr;        // AVPacket*
    int m_videoStreamIndex = -1;
    
    VideoMetadata m_metadata;
    bool m_hasMetadata = false;
    bool m_isOpen = false;
    bool m_useHardwareAcceleration = false;
    
    int64_t m_currentFrame = 0;
    int64_t m_currentTimestamp = 0;
    
    std::string m_filePath;
};

} // namespace aether
