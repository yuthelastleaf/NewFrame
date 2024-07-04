#pragma once

#include "stream_parser.h"

class video_stream_parser : public stream_parser
{
private:
public:
    video_stream_parser(AVFormatContext *video_format_ctx);
    virtual ~video_stream_parser();

public:
    int GetVideoWidth();
    int GetVideoHeight();
};
