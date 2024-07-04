#include "audio_stream_parser.h"

audio_stream_parser::audio_stream_parser(AVFormatContext *video_format_ctx)
    : stream_parser(video_format_ctx, AVMEDIA_TYPE_AUDIO)
{
}

audio_stream_parser::~audio_stream_parser()
{
}
