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

        window_ = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                   video_streams_->get_codec_context()->width, video_streams_->get_codec_context()->height, SDL_WINDOW_SHOWN);
        if (window_ == NULL)
        {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            break;
        }

        renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
        if (renderer_ == NULL)
        {
            printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
            break;
        }

        texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING,
                                     video_streams_->get_codec_context()->width, video_streams_->get_codec_context()->height);
        if (texture_ == NULL)
        {
            printf("Failed to create texture! SDL_Error: %s\n", SDL_GetError());
            break;
        }
        flag = true;

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
    AVCodecContext *pCodecCtx = video_streams_->get_codec_context();
    AVCodec *pCodec = video_streams_->get_codec();
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

    int64_t start_time = av_gettime();

    while (true)
    {
        AVPacket *packet = video_streams_->get_packet(1000);
        if (!packet)
        {
            if (!show_start_)
            {
                break;
            }
            else
            {
                continue;
            }
        }

        if (avcodec_send_packet(pCodecCtx, packet) >= 0)
        {
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
                

                // 计算播放进度
                int64_t current_time = av_gettime();
                int64_t elapsed_time = current_time - start_time;                                       // 经过的时间（单位：微秒）
                int progress_width = static_cast<int>((pCodecCtx->width * elapsed_time) / video_streams_->total_duration_); // 进度条的宽度

                // 绘制进度条背景
                SDL_Rect bgRect = {0, pCodecCtx->height, pCodecCtx->width, 50};
                SDL_SetRenderDrawColor(renderer_, 50, 50, 50, 255);
                SDL_RenderFillRect(renderer_, &bgRect);

                // 绘制进度条
                SDL_Rect progressRect = {0, pCodecCtx->height, progress_width, 50};
                SDL_SetRenderDrawColor(renderer_, 0, 255, 0, 255);
                SDL_RenderFillRect(renderer_, &progressRect);

                SDL_RenderPresent(renderer_);

                // 计算当前帧的显示时间
                int64_t pts = av_rescale_q(pOriFrame->pts, video_streams_->time_base_, AVRational{1, AV_TIME_BASE});
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

        av_packet_unref(packet);
        av_packet_free(&packet);
    }
}

void SDLShowVideo::ShowAudio()
{
    SDL_AudioSpec wanted_spec_, obtained_spec_;
    SwrContext *swr_ctx_;
    AVCodecContext *aCodecCtx = audio_streams_->get_codec_context();
    AVCodec *aCodec = audio_streams_->get_codec();
    if (!aCodecCtx || !aCodec)
    {
        fprintf(stderr, "Could not get codec context or codec\n");
        return;
    }

    swr_ctx_ = swr_alloc_set_opts(NULL, av_get_default_channel_layout(2), AV_SAMPLE_FMT_FLT, aCodecCtx->sample_rate,
                                  av_get_default_channel_layout(aCodecCtx->channels), aCodecCtx->sample_fmt, aCodecCtx->sample_rate, 0, NULL);
    if (!swr_ctx_ || swr_init(swr_ctx_) < 0)
    {
        fprintf(stderr, "Could not initialize the resampling context\n");
        return;
    }

    // 设置 SDL 音频规格
    SDL_zero(wanted_spec_);
    wanted_spec_.freq = aCodecCtx->sample_rate;
    wanted_spec_.format = AUDIO_F32SYS; // 使用浮点格式
    wanted_spec_.channels = 2;
    wanted_spec_.silence = 0;
    wanted_spec_.samples = 1024;
    wanted_spec_.callback = NULL;

    if (SDL_OpenAudio(&wanted_spec_, &obtained_spec_) < 0)
    {
        fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
        return;
    }

    // 开始播放音频
    SDL_PauseAudio(0);

    AVFrame *aFrame = av_frame_alloc();
    if (!aFrame)
    {
        fprintf(stderr, "Could not allocate audio frame\n");
        return;
    }

    int64_t audio_pts = 0; // 音频PTS
    // int64_t video_pts = 0; // 视频PTS
    int64_t start_time = av_gettime();

    while (show_start_)
    {
        AVPacket *packet = audio_streams_->get_packet(1000);
        if (!packet)
        {
            continue;
        }

        if (avcodec_send_packet(aCodecCtx, packet) >= 0)
        {
            while (avcodec_receive_frame(aCodecCtx, aFrame) == 0)
            {
                uint8_t *out_buffer = nullptr;
                int out_buffer_size = av_samples_get_buffer_size(NULL, 2, aFrame->nb_samples, AV_SAMPLE_FMT_FLT, 1);
                out_buffer = (uint8_t *)av_malloc(out_buffer_size);
                if (!out_buffer)
                {
                    fprintf(stderr, "Could not allocate output buffer\n");
                    continue;
                }

                int ret = swr_convert(swr_ctx_, &out_buffer, aFrame->nb_samples, (const uint8_t **)aFrame->extended_data, aFrame->nb_samples);
                if (ret < 0)
                {
                    fprintf(stderr, "Could not resample audio\n");
                    av_free(out_buffer);
                    continue;
                }

                SDL_QueueAudio(1, out_buffer, out_buffer_size);
                av_free(out_buffer);

                // 计算音频PTS
                audio_pts = av_rescale_q(aFrame->pts, audio_streams_->time_base_, AVRational{1, AV_TIME_BASE});
                int64_t delay = audio_pts - (av_gettime() - start_time);
                if (delay > 0)
                {
                    av_usleep(delay);
                }
            }
        }

        av_packet_unref(packet);
        av_packet_free(&packet);
    }

    SDL_CloseAudio();
    av_frame_free(&aFrame);
}

void SDLShowVideo::CreateSyncThread()
{
    sync_thread_.start<SDLShowVideo>(this, &SDLShowVideo::DispatchPacket);
    sync_thread_.start<SDLShowVideo>(this, &SDLShowVideo::ShowVideo);
    sync_thread_.start<SDLShowVideo>(this, &SDLShowVideo::ShowAudio);
}

void SDLShowVideo::DispatchPacket()
{
    while (true)
    {
        AVPacket *packet = av_packet_alloc(); // 分配AVPacket内存
        if (!packet)
        {
            std::cerr << "Failed to allocate AVPacket" << std::endl;
            continue;
        }
        if (av_read_frame(video_format_ctx_, packet) >= 0)
        {
            if (video_format_ctx_->streams[packet->stream_index]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
            {
                video_streams_->add_packet(packet);
            }
            else if (video_format_ctx_->streams[packet->stream_index]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
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

void SDLShowVideo::Start()
{
    InitSDL();
    show_start_ = true;
    CreateSyncThread();
}
