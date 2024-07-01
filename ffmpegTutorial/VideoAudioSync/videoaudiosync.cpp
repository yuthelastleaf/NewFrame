extern "C"
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>

#include <SDL.h>
#include <SDL_thread.h>
}

static Uint8 *audio_chunk;
static Uint32 audio_len;
static Uint8 *audio_pos;

#include <iostream>
// 初始化 SDL2
bool initSDL(SDL_Window **window, SDL_Renderer **renderer, int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
    {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    *window = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (*window == NULL)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (*renderer == NULL)
    {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

// 转换 AVSampleFormat 到 SDL_AudioFormat
SDL_AudioFormat get_sdl_audio_format(AVSampleFormat sample_fmt)
{
    switch (sample_fmt)
    {
    case AV_SAMPLE_FMT_U8:
        return AUDIO_U8;
    case AV_SAMPLE_FMT_S16:
        return AUDIO_S16SYS;
    case AV_SAMPLE_FMT_S32:
        return AUDIO_S32SYS;
    case AV_SAMPLE_FMT_FLT:
        return AUDIO_F32SYS;
    default:
        fprintf(stderr, "Unsupported audio format: %d\n", sample_fmt);
        return 0;
    }
}


// 音频回调函数
void audio_callback(void* userdata, Uint8* stream, int len)
{
    if (audio_len == 0) // 如果没有数据，则退出
        return;

    len = (len > audio_len ? audio_len : len); // 如果要求长度大于剩余数据长度，取较小值
    SDL_memcpy(stream, audio_pos, len);        // 复制音频数据到流
    audio_pos += len;
    audio_len -= len;
}



// 创建 YUV 纹理
SDL_Texture *createYUVTexture(SDL_Renderer *renderer, int width, int height)
{
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (texture == NULL)
    {
        printf("Failed to create texture! SDL_Error: %s\n", SDL_GetError());
    }
    return texture;
}

// Function to save a frame as a .ppm file
void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame)
{
    FILE *pFile;
    char szFilename[32];
    int y;

    sprintf(szFilename, "frame%d.ppm", iFrame);
    pFile = fopen(szFilename, "wb");
    if (pFile == NULL)
    {
        fprintf(stderr, "Could not open file %s\n", szFilename);
        return;
    }

    fprintf(pFile, "P6\n%d %d\n255\n", width, height);

    for (y = 0; y < height; y++)
        fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);

    fclose(pFile);
}

int main(int argc, char *argv[])
{
    AVFormatContext *pFormatCtx = NULL;
    int videoStream;
    int audioStream;
    AVCodecContext *pCodecCtxOrig = NULL;
    AVCodecContext *pCodecCtx = NULL;
    AVCodec *pCodec = NULL;

    AVCodecContext *paudio_CodecCtxOrig = NULL;
    AVCodecContext *paudio_CodecCtx = NULL;
    AVCodec *paudio_Codec = NULL;

    AVFrame *pFrame = NULL;
    AVFrame *pFrameRGB = NULL;
    AVPacket packet;
    int frameFinished;
    int numBytes;
    uint8_t *buffer = NULL;
    struct SwsContext *sws_ctx = NULL;
    char errbuf[128];
    int errnum;
    SwrContext* swr_ctx = NULL;

    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;

    if (argc < 2)
    {
        fprintf(stderr, "Please provide a movie file\n");
        return -1;
    }

    // Open video file
    if ((errnum = avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)) != 0)
    {
        av_strerror(errnum, errbuf, sizeof(errbuf));
        fprintf(stderr, "Could not open file %s: %s\n", argv[1], errbuf);
        return -1;
    }

    // Retrieve stream information
    if ((errnum = avformat_find_stream_info(pFormatCtx, NULL)) < 0)
    {
        av_strerror(errnum, errbuf, sizeof(errbuf));
        fprintf(stderr, "Could not find stream information: %s\n", errbuf);
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    // Dump information about file onto standard error
    av_dump_format(pFormatCtx, 0, argv[1], 0);

    // Find the first video stream
    videoStream = -1;
    audioStream = -1;
    for (int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && videoStream < 0)
        {
            videoStream = i;
        }
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && audioStream < 0)
        {
            audioStream = i;
        }
    }
    if (videoStream == -1 || audioStream == -1)
    {
        fprintf(stderr, "Did not find a video or audio stream\n");
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    // Get a pointer to the codec context for the video stream
    pCodecCtxOrig = avcodec_alloc_context3(NULL);
    paudio_CodecCtxOrig = avcodec_alloc_context3(NULL);
    if (!pCodecCtxOrig || !paudio_CodecCtxOrig)
    {
        fprintf(stderr, "Could not allocate video or audio codec context\n");
        avformat_close_input(&pFormatCtx);
        return -1;
    }
    if ((errnum = avcodec_parameters_to_context(pCodecCtxOrig, pFormatCtx->streams[videoStream]->codecpar)) < 0)
    {
        av_strerror(errnum, errbuf, sizeof(errbuf));
        fprintf(stderr, "Could not copy codec context: %s\n", errbuf);
        avcodec_free_context(&pCodecCtxOrig);
        avcodec_free_context(&paudio_CodecCtxOrig);
        avformat_close_input(&pFormatCtx);
        return -1;
    }
    // 初始化音频上下文
    if ((errnum = avcodec_parameters_to_context(paudio_CodecCtxOrig, pFormatCtx->streams[audioStream]->codecpar)) < 0)
    {
        av_strerror(errnum, errbuf, sizeof(errbuf));
        fprintf(stderr, "Could not copy codec context: %s\n", errbuf);
        avcodec_free_context(&pCodecCtxOrig);
        avcodec_free_context(&paudio_CodecCtxOrig);
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    // Find the decoder for the video stream
    pCodec = const_cast<AVCodec *>(avcodec_find_decoder(pCodecCtxOrig->codec_id));
    if (pCodec == NULL)
    {
        fprintf(stderr, "Unsupported codec!\n");
        avcodec_free_context(&pCodecCtxOrig);
        avcodec_free_context(&paudio_CodecCtxOrig);
        avformat_close_input(&pFormatCtx);
        return -1;
    }
    // Init audio codec
    paudio_Codec = const_cast<AVCodec *>(avcodec_find_decoder(paudio_CodecCtxOrig->codec_id));
    if (paudio_Codec == NULL)
    {
        fprintf(stderr, "Unsupported codec!\n");
        avcodec_free_context(&pCodecCtxOrig);
        avcodec_free_context(&paudio_CodecCtxOrig);
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    // Copy context
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if ((errnum = avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoStream]->codecpar)) < 0)
    {
        av_strerror(errnum, errbuf, sizeof(errbuf));
        fprintf(stderr, "Couldn't copy codec context: %s\n", errbuf);
        avcodec_free_context(&pCodecCtx);
        avcodec_free_context(&pCodecCtxOrig);
        avcodec_free_context(&paudio_CodecCtxOrig);
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    paudio_CodecCtx = avcodec_alloc_context3(paudio_Codec);
    if ((errnum = avcodec_parameters_to_context(paudio_CodecCtx, pFormatCtx->streams[audioStream]->codecpar)) < 0)
    {
        av_strerror(errnum, errbuf, sizeof(errbuf));
        fprintf(stderr, "Couldn't copy codec context: %s\n", errbuf);
        avcodec_free_context(&pCodecCtx);
        avcodec_free_context(&pCodecCtxOrig);
        avcodec_free_context(&paudio_CodecCtxOrig);
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    // Open codec
    if ((errnum = avcodec_open2(pCodecCtx, pCodec, NULL)) < 0)
    {
        av_strerror(errnum, errbuf, sizeof(errbuf));
        fprintf(stderr, "Could not open codec: %s\n", errbuf);
        avcodec_free_context(&pCodecCtx);
        avcodec_free_context(&pCodecCtxOrig);
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    // Open audio code
    if ((errnum = avcodec_open2(paudio_CodecCtx, paudio_Codec, NULL)) < 0)
    {
        av_strerror(errnum, errbuf, sizeof(errbuf));
        fprintf(stderr, "Could not open codec: %s\n", errbuf);
        avcodec_free_context(&pCodecCtx);
        avcodec_free_context(&pCodecCtxOrig);
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    // Allocate video frame
    pFrame = av_frame_alloc();
    if (!pFrame)
    {
        fprintf(stderr, "Could not allocate frame\n");
        avcodec_free_context(&pCodecCtx);
        avcodec_free_context(&pCodecCtxOrig);
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    // Allocate an AVFrame structure
    pFrameRGB = av_frame_alloc();
    if (pFrameRGB == NULL)
    {
        fprintf(stderr, "Could not allocate frame RGB\n");
        av_frame_free(&pFrame);
        avcodec_free_context(&pCodecCtx);
        avcodec_free_context(&pCodecCtxOrig);
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    // Determine required buffer size and allocate buffer
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
    buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    if (!buffer)
    {
        fprintf(stderr, "Could not allocate buffer\n");
        av_frame_free(&pFrameRGB);
        av_frame_free(&pFrame);
        avcodec_free_context(&pCodecCtx);
        avcodec_free_context(&pCodecCtxOrig);
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

    // 初始化 SDL2
    if (!initSDL(&window, &renderer, pCodecCtx->width, pCodecCtx->height))
    {
        av_free(buffer);
        av_frame_free(&pFrameRGB);
        av_frame_free(&pFrame);
        avcodec_free_context(&pCodecCtx);
        avcodec_free_context(&pCodecCtxOrig);
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    texture = createYUVTexture(renderer, pCodecCtx->width, pCodecCtx->height);
    if (texture == NULL)
    {
        printf("Failed to create texture\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // SDL音频规范
    SDL_AudioSpec wanted_spec, spec;
    wanted_spec.freq = paudio_CodecCtx->sample_rate;
    wanted_spec.format = AUDIO_F32SYS;
    wanted_spec.channels = paudio_CodecCtx->channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = 1024;
    wanted_spec.callback = audio_callback;
    wanted_spec.userdata = paudio_CodecCtx;

    if (SDL_OpenAudio(&wanted_spec, &spec) < 0)
    {
        fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
        return -1;
    }

    // initialize SWS context for software scaling
    sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                             pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR,
                             NULL, NULL, NULL);
    if (!sws_ctx)
    {
        fprintf(stderr, "Could not initialize the conversion context\n");
        av_free(buffer);
        av_frame_free(&pFrameRGB);
        av_frame_free(&pFrame);
        avcodec_free_context(&pCodecCtx);
        avcodec_free_context(&pCodecCtxOrig);
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    // 初始化音频重采样上下文
    swr_ctx = swr_alloc();
    if (!swr_ctx) {
        fprintf(stderr, "Could not allocate resampler context\n");
        return -1;
    }

    // 配置重采样上下文
    av_opt_set_int(swr_ctx, "in_channel_layout", paudio_CodecCtx->channel_layout, 0);
    av_opt_set_int(swr_ctx, "in_sample_rate", paudio_CodecCtx->sample_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", paudio_CodecCtx->sample_fmt, 0);

    av_opt_set_int(swr_ctx, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(swr_ctx, "out_sample_rate", paudio_CodecCtx->sample_rate, 0);
    av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_FLT, 0);  // 转换为交错浮点格式

    if ((errnum = swr_init(swr_ctx)) < 0)
    {
        av_strerror(errnum, errbuf, sizeof(errbuf));
        fprintf(stderr, "Failed to initialize the resampling context: %s\n", errbuf);
        av_free(buffer);
        av_frame_free(&pFrameRGB);
        av_frame_free(&pFrame);
        avcodec_free_context(&pCodecCtx);
        avcodec_free_context(&pCodecCtxOrig);
        avcodec_free_context(&paudio_CodecCtx);
        avcodec_free_context(&paudio_CodecCtxOrig);
        swr_free(&swr_ctx);
        avformat_close_input(&pFormatCtx);
        return -1;
    }


    // 获取视频流的时间基
    AVRational time_base = pFormatCtx->streams[videoStream]->time_base;
    AVRational audio_time_base = pFormatCtx->streams[audioStream]->time_base;

    int64_t start_time = av_gettime();

    // 开始播放音频
    SDL_PauseAudio(0);

    // Read frames and save first five frames to disk
    int i = 0;
    while (av_read_frame(pFormatCtx, &packet) >= 0)
    {
        // if (packet.stream_index == videoStream)
        if (pFormatCtx->streams[packet.stream_index]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            avcodec_send_packet(pCodecCtx, &packet);
            while (avcodec_receive_frame(pCodecCtx, pFrame) == 0)
            {
                // Convert the image from its native format to RGB
                sws_scale(sws_ctx, (uint8_t const *const *)pFrame->data,
                          pFrame->linesize, 0, pCodecCtx->height,
                          pFrameRGB->data, pFrameRGB->linesize);

                // 更新 YUV 纹理
                SDL_UpdateYUVTexture(
                    texture,
                    NULL,
                    pFrameRGB->data[0], pFrameRGB->linesize[0],
                    pFrameRGB->data[1], pFrameRGB->linesize[1],
                    pFrameRGB->data[2], pFrameRGB->linesize[2]);

                // 渲染纹理到屏幕
                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, texture, NULL, NULL);
                SDL_RenderPresent(renderer);

                // 计算当前帧的显示时间
                int64_t pts = av_rescale_q(pFrame->pts, time_base, AVRational{1, AV_TIME_BASE});
                int64_t delay = pts - (av_gettime() - start_time);
                if (delay > 0)
                {
                    av_usleep(delay);
                }

                // 处理事件（如退出）
                SDL_Event e;
                if (SDL_PollEvent(&e) != 0)
                {
                    if (e.type == SDL_QUIT)
                    {
                        break;
                    }
                }
            }
        }
        else if (pFormatCtx->streams[packet.stream_index]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {

            // 处理音频帧
            avcodec_send_packet(paudio_CodecCtx, &packet);
            while (avcodec_receive_frame(paudio_CodecCtx, pFrame) == 0) {
                uint8_t* output_buffer;
                int output_buffer_size;
                int output_samples;

                output_samples = av_rescale_rnd(swr_get_delay(swr_ctx, paudio_CodecCtx->sample_rate) + pFrame->nb_samples, paudio_CodecCtx->sample_rate, paudio_CodecCtx->sample_rate, AV_ROUND_UP);

                output_buffer_size = output_samples * av_get_bytes_per_sample(AV_SAMPLE_FMT_FLT) * paudio_CodecCtx->channels;
                output_buffer = (uint8_t*)av_malloc(output_buffer_size);

                int ret = swr_convert(swr_ctx, &output_buffer, output_samples, (const uint8_t**)pFrame->extended_data, pFrame->nb_samples);
                if (ret < 0) {
                    fprintf(stderr, "Error while converting audio\n");
                    av_free(output_buffer);
                    continue;
                }

                audio_chunk = output_buffer;
                audio_len = output_buffer_size;
                audio_pos = audio_chunk;

                int64_t pts = av_rescale_q(pFrame->pts, audio_time_base, AVRational{ 1, AV_TIME_BASE });
                int64_t delay = pts - (av_gettime() - start_time);
                if (delay > 0) {
                    av_usleep(delay);
                }

                av_free(output_buffer);
            }
        }
        av_packet_unref(&packet);

        // Convert the image from its native format to RGB
        /*sws_scale(sws_ctx, (uint8_t const *const *)pFrame->data,
                  pFrame->linesize, 0, pCodecCtx->height,
                  pFrameRGB->data, pFrameRGB->linesize);*/
    }

    // Free the RGB image
    av_free(buffer);
    av_frame_free(&pFrameRGB);

    // Free the YUV frame
    av_frame_free(&pFrame);

    // Close the codecs
    avcodec_close(pCodecCtx);
    avcodec_free_context(&pCodecCtx);
    avcodec_free_context(&pCodecCtxOrig);

    // Close the video file
    avformat_close_input(&pFormatCtx);

    sws_freeContext(sws_ctx);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
