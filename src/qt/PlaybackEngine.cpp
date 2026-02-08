#include "aether/PlaybackEngine.h"
#include "aether/FrameRingBuffer.h"
#include <QThread>
#include <QTimer>
#include <cstring>

#ifdef AETHER_FFMPEG_ENABLED
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}
#endif

namespace aether {

#ifdef AETHER_FFMPEG_ENABLED
class DecodeThread : public QThread {
public:
    DecodeThread(const QString& path, FrameRingBuffer* ring, std::atomic<bool>* playing,
                 std::atomic<bool>* seekRequested, std::atomic<qint64>* seekTargetMs,
                 std::atomic<qint64>* currentTimeMs, qint64* durationMs, QString* errorMsg)
        : m_path(path), m_ring(ring), m_playing(playing), m_seekRequested(seekRequested)
        , m_seekTargetMs(seekTargetMs), m_currentTimeMs(currentTimeMs), m_durationMs(durationMs)
        , m_errorMsg(errorMsg) {}

    void run() override {
        AVFormatContext* fmt = nullptr;
        AVCodecContext* codec = nullptr;
        const AVCodec* dec = nullptr;
        AVFrame* frame = nullptr;
        AVPacket* pkt = nullptr;
        SwsContext* sws = nullptr;
        int videoStream = -1;
        AVRational timeBase;
        timeBase.num = 1;
        timeBase.den = 30;

        if (avformat_open_input(&fmt, m_path.toUtf8().constData(), nullptr, nullptr) < 0) {
            if (m_errorMsg) *m_errorMsg = QStringLiteral("Could not open file");
            return;
        }
        if (avformat_find_stream_info(fmt, nullptr) < 0) {
            avformat_close_input(&fmt);
            if (m_errorMsg) *m_errorMsg = QStringLiteral("Could not find stream info");
            return;
        }
        for (unsigned i = 0; i < fmt->nb_streams; i++) {
            if (fmt->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                videoStream = static_cast<int>(i);
                timeBase = fmt->streams[i]->time_base;
                break;
            }
        }
        if (videoStream < 0) {
            avformat_close_input(&fmt);
            if (m_errorMsg) *m_errorMsg = QStringLiteral("No video stream");
            return;
        }
        *m_durationMs = (fmt->duration * 1000 * timeBase.num) / timeBase.den;
        if (*m_durationMs <= 0 && fmt->duration != AV_NOPTS_VALUE)
            *m_durationMs = (fmt->duration * 1000) / AV_TIME_BASE;

        dec = avcodec_find_decoder(fmt->streams[videoStream]->codecpar->codec_id);
        if (!dec) {
            avformat_close_input(&fmt);
            if (m_errorMsg) *m_errorMsg = QStringLiteral("Codec not found");
            return;
        }
        codec = avcodec_alloc_context3(dec);
        avcodec_parameters_to_context(codec, fmt->streams[videoStream]->codecpar);
        if (avcodec_open2(codec, dec, nullptr) < 0) {
            avcodec_free_context(&codec);
            avformat_close_input(&fmt);
            if (m_errorMsg) *m_errorMsg = QStringLiteral("Could not open codec");
            return;
        }
        frame = av_frame_alloc();
        pkt = av_packet_alloc();
        int w = codec->width;
        int h = codec->height;
        sws = sws_getContext(w, h, codec->pix_fmt, w, h, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);

        while (!isInterruptionRequested()) {
            if (m_seekRequested->exchange(false)) {
                qint64 target = m_seekTargetMs->load();
                int64_t ts = (target * timeBase.den) / (1000 * timeBase.num);
                av_seek_frame(fmt, videoStream, ts, AVSEEK_FLAG_BACKWARD);
                avcodec_flush_buffers(codec);
                *m_currentTimeMs = target;
            }
            if (!m_playing->load()) {
                msleep(20);
                continue;
            }
            if (av_read_frame(fmt, pkt) < 0) {
                av_packet_unref(pkt);
                msleep(10);
                continue;
            }
            if (pkt->stream_index != videoStream) {
                av_packet_unref(pkt);
                continue;
            }
            int ret = avcodec_send_packet(codec, pkt);
            av_packet_unref(pkt);
            if (ret < 0) continue;
            while (avcodec_receive_frame(codec, frame) == 0) {
                qint64 ptsMs = (frame->pts * 1000 * timeBase.num) / timeBase.den;
                if (ptsMs < 0) ptsMs = 0;
                *m_currentTimeMs = ptsMs;

                int linesize = frame->linesize[0];
                int size = w * h * 3;
                std::vector<uint8_t> rgb(static_cast<size_t>(size));
                uint8_t* dst[1] = { rgb.data() };
                int dstStride[1] = { w * 3 };
                sws_scale(sws, frame->data, frame->linesize, 0, h, dst, dstStride);
                if (m_ring)
                    m_ring->push(w, h, rgb.data(), static_cast<size_t>(size), ptsMs);
            }
            msleep(1);
        }

        if (sws) sws_freeContext(sws);
        av_packet_free(&pkt);
        av_frame_free(&frame);
        avcodec_free_context(&codec);
        avformat_close_input(&fmt);
    }

private:
    QString m_path;
    FrameRingBuffer* m_ring;
    std::atomic<bool>* m_playing;
    std::atomic<bool>* m_seekRequested;
    std::atomic<qint64>* m_seekTargetMs;
    std::atomic<qint64>* m_currentTimeMs;
    qint64* m_durationMs;
    QString* m_errorMsg;
};
#endif

PlaybackEngine::PlaybackEngine(QObject* parent) : QObject(parent) {
    m_ringBuffer = std::make_unique<FrameRingBuffer>(8);
    m_pollTimer = new QTimer(this);
    m_pollTimer->setInterval(33);
    connect(m_pollTimer, &QTimer::timeout, this, &PlaybackEngine::pollFrame);
}

PlaybackEngine::~PlaybackEngine() {
    stop();
#ifdef AETHER_FFMPEG_ENABLED
    if (m_decodeThread) {
        m_decodeThread->requestInterruption();
        m_decodeThread->wait(2000);
        delete m_decodeThread;
        m_decodeThread = nullptr;
    }
#endif
}

void PlaybackEngine::setSource(const QString& path) {
    stop();
    m_sourcePath = path;
    m_durationMs = 0;
    m_currentTimeMs = 0;
    m_durationEmitted = false;
    m_ringBuffer->clear();
#ifdef AETHER_FFMPEG_ENABLED
    if (!path.isEmpty()) {
        QString err;
        m_decodeThread = new DecodeThread(path, m_ringBuffer.get(), &m_playing, &m_seekRequested,
                                         &m_seekTargetMs, &m_currentTimeMs, &m_durationMs, &err);
        m_decodeThread->start();
        if (!err.isEmpty())
            emit errorOccurred(err);
        else
            emit durationChanged(m_durationMs);
    }
#endif
}

void PlaybackEngine::play() {
    m_playing = true;
    m_pollTimer->start();
}

void PlaybackEngine::pause() {
    m_playing = false;
    m_pollTimer->stop();
}

void PlaybackEngine::stop() {
    m_playing = false;
    m_pollTimer->stop();
#ifdef AETHER_FFMPEG_ENABLED
    if (m_decodeThread) {
        m_decodeThread->requestInterruption();
        m_decodeThread->wait(3000);
        delete m_decodeThread;
        m_decodeThread = nullptr;
    }
#endif
}

void PlaybackEngine::seek(qint64 positionMs) {
    m_seekTargetMs = positionMs;
    m_seekRequested = true;
    m_currentTimeMs = positionMs;
    emit positionChanged(positionMs);
}

qint64 PlaybackEngine::getCurrentTimeMs() const {
    return m_currentTimeMs.load();
}

qint64 PlaybackEngine::getDurationMs() const {
    return m_durationMs;
}

QImage PlaybackEngine::getCurrentFrame() const {
    DecodedFrame frame;
    if (!m_ringBuffer->getLatest(frame) || frame.width <= 0 || frame.height <= 0)
        return QImage();
    QImage img(frame.width, frame.height, QImage::Format_RGB888);
    size_t copySize = static_cast<size_t>(frame.width * frame.height * 3);
    if (frame.rgb.size() >= copySize)
        std::memcpy(img.bits(), frame.rgb.data(), copySize);
    return img;
}

void PlaybackEngine::pollFrame() {
    if (m_durationMs > 0 && !m_durationEmitted) {
        m_durationEmitted = true;
        emit durationChanged(m_durationMs);
    }
    DecodedFrame frame;
    if (m_ringBuffer->getLatest(frame))
        emit frameReady();
    emit positionChanged(m_currentTimeMs.load());
}

} // namespace aether
