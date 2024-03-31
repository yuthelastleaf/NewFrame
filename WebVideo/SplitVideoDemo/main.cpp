// 确保包含FFmpeg库
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
}

int main() {
    const char* in_filename = "input.mp4";
    const char* out_filename = "output.m3u8"; // HLS播放列表文件

    AVFormatContext* ifmt_ctx = nullptr;
    AVFormatContext* ofmt_ctx = nullptr;

    // 初始化网络（如果需要）
    avformat_network_init();

    // 打开输入文件
    if (avformat_open_input(&ifmt_ctx, in_filename, nullptr, nullptr) < 0) {
        fprintf(stderr, "Could not open input file\n");
        return -1;
    }

    // 创建输出上下文，指定输出格式为"hls"
    if (avformat_alloc_output_context2(&ofmt_ctx, nullptr, "hls", out_filename) < 0) {
        fprintf(stderr, "Could not create output context\n");
        return -1;
    }

    // 下面是复制输入流到输出流的代码（省略）
    // 循环遍历每个输入流
    for (int i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            fprintf(stderr, "Failed allocating output stream\n");
            break;
        }

        // 复制输入流的编解码器参数到输出流
        int ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy codec parameters\n");
            break;
        }
        out_stream->codecpar->codec_tag = 0;
    }

    // 特别针对HLS的设置
    av_opt_set(ofmt_ctx->priv_data, "hls_time", "4", 0); // 每个TS文件的最大时长（秒）
    av_opt_set(ofmt_ctx->priv_data, "hls_list_size", "0", 0); // 保留的播放列表中的TS文件数量，0表示不限制
    av_opt_set(ofmt_ctx->priv_data, "hls_segment_filename", "stream_%03d.ts", 0); // TS文件的命名规则

    // 打开输出文件（对于HLS，这实际上创建了M3U8文件）
    if (avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE) < 0) {
        fprintf(stderr, "Could not open output file '%s'\n", out_filename);
        return -1;
    }

    // 写入文件头（对于HLS，这会初始化M3U8文件）
    if (avformat_write_header(ofmt_ctx, nullptr) < 0) {
        fprintf(stderr, "Error occurred when opening output file\n");
        return -1;
    }

    // 处理每个帧的复制或转码（省略）
    // 初始化数据包结构
    AVPacket pkt;
    av_init_packet(&pkt);

    // 循环读取输入文件的数据包
    while (av_read_frame(ifmt_ctx, &pkt) >= 0) {
        AVStream* in_stream, * out_stream;

        // 获取输入和输出流
        in_stream = ifmt_ctx->streams[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];

        // 复制数据包
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;

        // 将数据包写入输出文件
        if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) {
            fprintf(stderr, "Error muxing packet\n");
            break;
        }

        // 释放数据包以避免内存泄漏
        av_packet_unref(&pkt);
    }

    // 最后不要忘记释放pkt
    av_packet_unref(&pkt);

    // 写入文件尾（对于HLS，这会完成M3U8文件）
    av_write_trailer(ofmt_ctx);

    // 清理
    avformat_close_input(&ifmt_ctx);
    if (ofmt_ctx && !(ofmt_ctx->oformat->flags & AVFMT_NOFILE)) {
        avio_closep(&ofmt_ctx->pb);
    }
    avformat_free_context(ofmt_ctx);

    return 0;
}
