#ifndef FF_CMD_H_
#define FF_CMD_H_

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/channel_layout.h>
#include <libavutil/mem.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
}

class FFCmd
{
private:
    /* data */
    AVFormatContext *ifmt_ctx; // 输入的数据流

    
public:
    FFCmd(/* args */const char* filename);
    ~FFCmd();

public:
    int open_input_file(const char *filename);

    void av_show_info();
};


#endif // FF_CMD_H_
