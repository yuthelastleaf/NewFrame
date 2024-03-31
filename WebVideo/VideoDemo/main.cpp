extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}
int main(int argc, char **argv) {
    AVFormatContext *input_format_context = NULL, *output_format_context = NULL;
    AVCodecContext *input_codec_context = NULL, *output_codec_context = NULL;
    AVStream *video_stream = NULL;
    AVCodec *input_codec = NULL;
    const AVCodec *output_codec = NULL;
    AVPacket packet;
    int video_stream_index = -1;
    int ret;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        return -1;
    }

    // 打开输入文件
    if (avformat_open_input(&input_format_context, argv[1], NULL, NULL) < 0) {
        fprintf(stderr, "Could not open input file '%s'\n", argv[1]);
        return -1;
    }

    if (avformat_find_stream_info(input_format_context, NULL) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information\n");
        return -1;
    }

    // 查找视频流
    for (int i = 0; i < input_format_context->nb_streams; i++) {
        if (input_format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }

    if (video_stream_index == -1) {
        fprintf(stderr, "No video stream found\n");
        return -1;
    }

    // 打开输出文件
    avformat_alloc_output_context2(&output_format_context, NULL, NULL, argv[2]);
    if (!output_format_context) {
        fprintf(stderr, "Could not create output context\n");
        return -1;
    }

    // 创建输出流
    video_stream = avformat_new_stream(output_format_context, NULL);
    if (!video_stream) {
        fprintf(stderr, "Failed to create output stream\n");
        return -1;
    }

    // 设置编解码器上下文参数（这里简化处理，实际应用中可能需要更多设置）
    input_codec_context = avcodec_alloc_context3(input_codec);
    avcodec_parameters_to_context(input_codec_context, input_format_context->streams[video_stream_index]->codecpar);

    output_codec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);
    if (!output_codec) {
        fprintf(stderr, "Codec not found\n");
        return -1;
    }

    output_codec_context = avcodec_alloc_context3(output_codec);
    output_codec_context->bit_rate = input_codec_context->bit_rate; // 示例：复制比特率
    output_codec_context->width = input_codec_context->width; // 示例：复制宽度
    output_codec_context->height = input_codec_context->height; // 示例：复制高度
    output_codec_context->pix_fmt = output_codec->pix_fmts[0]; // 示例：使用编解码器推荐的像素格式
    output_codec_context->time_base = input_codec_context->time_base; // 示例：复制时间基

    int openres = avcodec_open2(output_codec_context, output_codec, NULL);
    if (openres < 0) {
        char err_buf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(openres, err_buf, sizeof(err_buf));
        fprintf(stderr, "Could not open codec: %s\n", err_buf);
        return -1;
    }

    avcodec_parameters_from_context(video_stream->codecpar, output_codec_context);

    if (!(output_format_context->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&output_format_context->pb, argv[2], AVIO_FLAG_WRITE) < 0) {
            fprintf(stderr, "Could not open output file '%s'\n", argv[2]);
            return -1;
        }
    }

    // 写入文件头信息
    if (avformat_write_header(output_format_context, NULL) < 0) {
        fprintf(stderr, "Error occurred when opening output file\n");
        return -1;
    }

    // 转换数据（这里简化处理，实际应用中可能需要对帧进行编解码处理）
    while (av_read_frame(input_format_context, &packet) >= 0) {
        if (packet.stream_index == video_stream_index) {
            packet.stream_index = video_stream->index;
            av_interleaved_write_frame(output_format_context, &packet);
        }
        av_packet_unref(&packet);
    }

    // 写入文件尾信息
    av_write_trailer(output_format_context);

    // 清理
    avcodec_free_context(&input_codec_context);
    avcodec_free_context(&output_codec_context);
    avformat_close_input(&input_format_context);
    avformat_free_context(output_format_context);

    return 0;
}
