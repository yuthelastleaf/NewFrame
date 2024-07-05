#include "stream_parser.h"

stream_parser::stream_parser(AVFormatContext *format_ctx, AVMediaType stream_type)
    : is_init_(false)
{

    for (int i = 0; i < format_ctx->nb_streams; i++)
    {
        if (format_ctx->streams[i]->codecpar->codec_type == stream_type)
        {
            int errnum;
            char errbuf[128];
            AVCodecContext *pCodecCtxOrig = avcodec_alloc_context3(NULL);
            if (!pCodecCtxOrig)
            {
                fprintf(stderr, "Could not allocate video or audio codec context\n");
                break;
            }
            if ((errnum = avcodec_parameters_to_context(pCodecCtxOrig, format_ctx->streams[i]->codecpar)) < 0)
            {
                av_strerror(errnum, errbuf, sizeof(errbuf));
                fprintf(stderr, "Could not copy codec context: %s\n", errbuf);
                break;
            }
            stream_codec_ = const_cast<AVCodec *>(avcodec_find_decoder(pCodecCtxOrig->codec_id));
            if (stream_codec_ == NULL)
            {
                fprintf(stderr, "Unsupported codec!\n");
                break;
            }

            // Copy context
            stream_codec_ctx_ = avcodec_alloc_context3(stream_codec_);
            if ((errnum = avcodec_parameters_to_context(stream_codec_ctx_, format_ctx->streams[i]->codecpar)) < 0)
            {
                av_strerror(errnum, errbuf, sizeof(errbuf));
                fprintf(stderr, "Couldn't copy codec context: %s\n", errbuf);
                break;
            }

            // Open codec
            if ((errnum = avcodec_open2(stream_codec_ctx_, stream_codec_, NULL)) < 0)
            {
                av_strerror(errnum, errbuf, sizeof(errbuf));
                fprintf(stderr, "Could not open codec: %s\n", errbuf);
                break;
            }

            is_init_ = true;
            break;
        }
    }
}

stream_parser::~stream_parser()
{
    if (stream_codec_ctx_)
    {
        avcodec_free_context(&stream_codec_ctx_);
        stream_codec_ = NULL;
        stream_codec_ctx_ = NULL;
    }
}

void stream_parser::add_packet(AVPacket& packet)
{
    semaphore_.signal(&packet);
}

AVPacket *stream_parser::get_packet(int timeout)
{
    AVPacket* packet = nullptr;
    if(!timeout) {
        packet = semaphore_.try_wait();
    } else {
        packet = semaphore_.timed_wait(timeout);
    }
    return packet;
}

AVCodecContext *stream_parser::get_codec_context()
{
    return stream_codec_ctx_;
}

AVCodec *stream_parser::get_codec()
{
    return stream_codec_;
}
