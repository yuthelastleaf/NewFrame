#include "SDLShowVideo.h"

SDLShowVideo::SDLShowVideo()
    : texture_(NULL), window_(NULL), renderer_(NULL), video_file_(NULL), video_format_ctx_(NULL), show_start_(false)
{
}

SDLShowVideo::SDLShowVideo(char *filename)
    : texture_(NULL), window_(NULL), renderer_(NULL), video_file_(NULL), video_format_ctx_(NULL), show_start_(false)
{
    ReadVideoFile(filename);
}

SDLShowVideo::~SDLShowVideo()
{
    if (video_format_ctx_)
    {
        avformat_close_input(&video_format_ctx_);
    }
    if (texture_)
    {
        SDL_DestroyTexture(texture_);
    }
    if (renderer_)
    {
        SDL_DestroyRenderer(renderer_);
    }
    if (window_)
    {
        SDL_DestroyWindow(window_);
    }
    SDL_Quit();
}

bool SDLShowVideo::InitSDL()
{
    bool flag = false;

    do
    {
        if (!video_streams_)
        {
            printf("There is no video stream\n");
            break;
        }

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) < 0)
        {
            printf("SDL init failed! SDL_Error: %s\n", SDL_GetError());
            break;
        }

        window_ = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 100, 100, SDL_WINDOW_SHOWN);
        if (window_ == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            return false;
        }

        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
        if (renderer_ == NULL)
        {
            printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
            return false;
        }
    } while (0);

    return flag;
}

bool SDLShowVideo::ReadVideoFile(char *filename)
{
    int errnum;
    char errbuf[128];
    bool flag = false;
    do
    {
        if ((errnum = avformat_open_input(&video_format_ctx_, filename, NULL, NULL)) != 0)
        {
            av_strerror(errnum, errbuf, sizeof(errbuf));
            fprintf(stderr, "Could not open file %s: %s\n", filename, errbuf);
            break;
        }

        // Retrieve stream information
        if ((errnum = avformat_find_stream_info(video_format_ctx_, NULL)) < 0)
        {
            av_strerror(errnum, errbuf, sizeof(errbuf));
            fprintf(stderr, "Could not find stream information: %s\n", errbuf);
            break;
        }
        av_dump_format(video_format_ctx_, 0, filename, 0);

        video_streams_ = std::make_unique<stream_parser>(video_format_ctx_, AVMEDIA_TYPE_VIDEO);
        audio_streams_ = std::make_unique<stream_parser>(video_format_ctx_, AVMEDIA_TYPE_AUDIO);

        flag = true;

    } while (0);

    return flag;
}

void SDLShowVideo::ShowVideo()
{
    int numBytes;
    uint8_t *buffer = NULL;
    struct SwsContext *sws_ctx = NULL;
    AVCodecContext* pCodecCtx = video_streams_->get_codec_context();
    AVCodec*        pCodec = video_streams_->get_codec();
    // Allocate video frame
    AVFrame *pOriFrame = av_frame_alloc();
    AVFrame *pDstFrame = av_frame_alloc();
    if (!pOriFrame || !pDstFrame)
    {
        fprintf(stderr, "Could not allocate frame\n");
        return;
    }
    sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                             pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR,
                             NULL, NULL, NULL);
    if (!sws_ctx)
    {
        fprintf(stderr, "Could not initialize the conversion context\n");
        return;
    }
    
    // Determine required buffer size and allocate buffer
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
    buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));
    if (!buffer)
    {
        fprintf(stderr, "Could not allocate buffer\n");
        return;
    }

    // Assign appropriate parts of buffer to image planes in pFrameRGB
    av_image_fill_arrays(pDstFrame->data, pDstFrame->linesize, buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);

    // 获取视频流的时间基
    AVRational time_base = pFormatCtx->streams[videoStream]->time_base;
    int64_t start_time = av_gettime();

    while (true)
    {
        AVPacket* packet = video_streams_->get_packet(1000);
        if(!packet) {
            if(!show_start_) {
                break;
            } else {
                continue;
            }
        }

        avcodec_send_packet(pCodecCtx, packet);
        while (avcodec_receive_frame(pCodecCtx, pOriFrame) == 0)
        {
            // Convert the image from its native format to RGB
            sws_scale(sws_ctx, (uint8_t const *const *)pOriFrame->data,
                      pOriFrame->linesize, 0, pCodecCtx->height,
                      pDstFrame->data, pDstFrame->linesize);

            // 更新 YUV 纹理
            SDL_UpdateYUVTexture(
                texture_,
                NULL,
                pDstFrame->data[0], pDstFrame->linesize[0],
                pDstFrame->data[1], pDstFrame->linesize[1],
                pDstFrame->data[2], pDstFrame->linesize[2]);

            // 渲染纹理到屏幕
            SDL_RenderClear(renderer_);
            SDL_RenderCopy(renderer_, texture_, NULL, NULL);
            SDL_RenderPresent(renderer_);

            // 计算当前帧的显示时间
            int64_t pts = av_rescale_q(pOriFrame->pts, time_base, AVRational{1, AV_TIME_BASE});
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
}

void SDLShowVideo::Start()
{
    bool flag = true;
    while (true)
    {
        AVPacket packet;
        if (av_read_frame(video_format_ctx_, &packet) >= 0)
        {
            if (video_format_ctx_->streams[packet.stream_index]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                video_streams_->add_packet(packet);
            }
            else if (video_format_ctx_->streams[packet.stream_index]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                audio_streams_->add_packet(packet);
            }
        }
        else
        {
            break;
        }
    }
}
