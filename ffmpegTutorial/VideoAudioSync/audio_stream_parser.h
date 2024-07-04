#pragma once

#include "stream_parser.h"

class audio_stream_parser : public stream_parser
{
private:
    /* data */
public:
    audio_stream_parser(AVFormatContext *video_format_ctx);
    ~audio_stream_parser();
};

