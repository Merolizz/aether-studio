#include "../../include/aether/VideoLoader.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>


#ifdef AETHER_FFMPEG_ENABLED
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>

}
#else
extern "C" {
struct AVFormatContext;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
struct AVCodec;
}
#endif

namespace aether {

VideoLoader::VideoLoader() {}

VideoLoader::~VideoLoader() { close(); }

bool VideoLoader::open(const std::string &filePath) {
  if (m_isOpen) {
    close();
  }

  m_filePath = filePath;

  // Check if file exists
  std::ifstream file(filePath);
  if (!file.good()) {
    std::cerr << "Video file does not exist: " << filePath << std::endl;
    return false;
  }
  file.close();

  // Initialize decoder
  if (!initializeDecoder()) {
    std::cerr << "Failed to initialize video decoder" << std::endl;
    return false;
  }

  // Placeholder metadata
  m_metadata.width = 1920;
  m_metadata.height = 1080;
  m_metadata.fps = 30;
  m_metadata.duration = 10000000;
  m_metadata.codec = "h264";
  m_metadata.pixelFormat = PixelFormat::YUV420P;
  m_metadata.colorSpace = ColorSpace::BT709;
  m_hasMetadata = true;

  m_isOpen = true;
  m_currentFrame = 0;
  m_currentTimestamp = 0;

  std::cout << "Video opened: " << filePath << std::endl;
  return true;
}

void VideoLoader::close() {
  if (!m_isOpen) {
    return;
  }

  cleanupDecoder();

  m_isOpen = false;
  m_hasMetadata = false;
  m_currentFrame = 0;
  m_currentTimestamp = 0;
  m_filePath.clear();

  std::cout << "Video closed" << std::endl;
}

bool VideoLoader::readFrame(VideoFrame &frame) {
  if (!m_isOpen) {
    return false;
  }

  frame.width = m_metadata.width;
  frame.height = m_metadata.height;
  frame.format = m_metadata.pixelFormat;
  frame.colorSpace = m_metadata.colorSpace;
  frame.timestamp = m_currentTimestamp;
  frame.frameNumber = m_currentFrame;

  size_t frameSize = frame.width * frame.height * 3;
  frame.data.resize(frameSize, 0);

  m_currentFrame++;
  m_currentTimestamp += (1000000 / m_metadata.fps);

  return true;
}

bool VideoLoader::seekToFrame(int64_t frameNumber) {
  if (!m_isOpen) {
    return false;
  }

  if (frameNumber < 0 ||
      frameNumber >= (m_metadata.duration * m_metadata.fps / 1000000)) {
    return false;
  }

  m_currentFrame = frameNumber;
  m_currentTimestamp = (frameNumber * 1000000) / m_metadata.fps;

  return true;
}

bool VideoLoader::seekToTime(int64_t timestamp) {
  if (!m_isOpen) {
    return false;
  }

  if (timestamp < 0 || timestamp > m_metadata.duration) {
    return false;
  }

  m_currentTimestamp = timestamp;
  m_currentFrame = (timestamp * m_metadata.fps) / 1000000;

  return true;
}

bool VideoLoader::seekToPosition(double position) {
  if (position < 0.0 || position > 1.0) {
    return false;
  }

  int64_t targetTimestamp =
      static_cast<int64_t>(m_metadata.duration * position);
  return seekToTime(targetTimestamp);
}

double VideoLoader::getCurrentPosition() const {
  if (!m_isOpen || m_metadata.duration == 0) {
    return 0.0;
  }

  return static_cast<double>(m_currentTimestamp) /
         static_cast<double>(m_metadata.duration);
}

bool VideoLoader::enableHardwareAcceleration(bool enable) {
  m_useHardwareAcceleration = enable;

  if (m_isOpen) {
    cleanupDecoder();
    return initializeDecoder();
  }

  return true;
}

bool VideoLoader::initializeDecoder() { return true; }

void VideoLoader::cleanupDecoder() {}

bool VideoLoader::decodeFrame(VideoFrame &frame) {
  (void)frame;
  return false;
}

PixelFormat VideoLoader::convertPixelFormat(int avFormat) {
  (void)avFormat;
  return PixelFormat::RGB8;
}

ColorSpace VideoLoader::detectColorSpace() { return ColorSpace::BT709; }

bool VideoLoader::isFormatSupported(const std::string &filePath) {
  std::string ext = filePath.substr(filePath.find_last_of(".") + 1);

  std::vector<std::string> supported = {"mp4",  "mov", "avi", "mkv",
                                        "webm", "m4v", "mpg", "mpeg",
                                        "flv",  "wmv", "3gp"};

  for (const auto &format : supported) {
    if (ext == format) {
      return true;
    }
  }

  return false;
}

std::vector<std::string> VideoLoader::getSupportedFormats() {
  return {"mp4", "mov",  "avi", "mkv", "webm", "m4v",
          "mpg", "mpeg", "flv", "wmv", "3gp"};
}

} // namespace aether
