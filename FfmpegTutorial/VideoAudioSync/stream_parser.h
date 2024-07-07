#pragma once

extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
}

#include <queue>

#include "CppSampher.h"

class stream_parser
{
private:
    bool is_init_;

protected:
    /* data */
    AVCodecContext *stream_codec_ctx_;
    AVCodec *stream_codec_;
    Semaphore<AVPacket> semaphore_;

public:
    AVRational time_base_;
    int64_t total_duration_;
public:
    stream_parser(AVFormatContext *format_ctx, AVMediaType stream_type);
    virtual ~stream_parser();

    void add_packet(AVPacket* packet);
    AVPacket* get_packet(int timeout = 0);

    AVCodecContext* get_codec_context();
    AVCodec* get_codec();
};


