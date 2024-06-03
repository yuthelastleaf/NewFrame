#include "FFCmd.h"

FFCmd::FFCmd(const char *filename)
{
    open_input_file(filename);
}

FFCmd::~FFCmd()
{
    // 关闭输入文件并释放上下文
    avformat_close_input(&ifmt_ctx);
}

int FFCmd::open_input_file(const char *filename)
{
    int ret;
    unsigned int i;

    ifmt_ctx = NULL;
    if ((ret = avformat_open_input(&ifmt_ctx, filename, NULL, NULL)) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot open input file\n");
        return ret;
    }

    if ((ret = avformat_find_stream_info(ifmt_ctx, NULL)) < 0)
    {
        av_log(NULL, AV_LOG_ERROR, "Cannot find stream information\n");
        return ret;
    }

    av_dump_format(ifmt_ctx, 0, filename, 0);
    return 0;
}

void FFCmd::av_show_info()
{
    AVDictionaryEntry *tag = NULL;
    // 遍历并打印元数据
    printf("\nMetadata:\n");
    while ((tag = av_dict_get(ifmt_ctx->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
    {
        printf("  %s: %s\n", tag->key, tag->value);
    }
}
