#include "video_stream_parser.h"

video_stream_parser::video_stream_parser(AVFormatContext *video_format_ctx)
    : stream_parser(video_format_ctx, AVMEDIA_TYPE_VIDEO)
{

}

video_stream_parser::~video_stream_parser()
{
}

int video_stream_parser::GetVideoWidth()
{
    int width = 0;
    if(stream_codec_ctx_) {
        width = stream_codec_ctx_->width;
    }
    return width;
}

int video_stream_parser::GetVideoHeight()
{
    int height = 0;
    if(stream_codec_ctx_) {
        height = stream_codec_ctx_->height;
    }
    return height;
}
