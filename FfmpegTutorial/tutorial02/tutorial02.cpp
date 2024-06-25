extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

#include <SDL.h>
#include <SDL_thread.h>
}

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

int main(int argc, char* argv[])
{
    AVFormatContext *pFormatCtx = NULL;
    int videoStream;
    AVCodecContext *pCodecCtxOrig = NULL;
    AVCodecContext *pCodecCtx = NULL;
    AVCodec *pCodec = NULL;
    AVFrame *pFrame = NULL;
    AVFrame *pFrameRGB = NULL;
    AVPacket packet;
    int frameFinished;
    int numBytes;
    uint8_t *buffer = NULL;
    struct SwsContext *sws_ctx = NULL;
    char errbuf[128];
    int errnum;

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
    for (int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoStream = i;
            break;
        }
    }
    if (videoStream == -1)
    {
        fprintf(stderr, "Did not find a video stream\n");
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    // Get a pointer to the codec context for the video stream
    pCodecCtxOrig = avcodec_alloc_context3(NULL);
    if (!pCodecCtxOrig)
    {
        fprintf(stderr, "Could not allocate codec context\n");
        avformat_close_input(&pFormatCtx);
        return -1;
    }
    if ((errnum = avcodec_parameters_to_context(pCodecCtxOrig, pFormatCtx->streams[videoStream]->codecpar)) < 0)
    {
        av_strerror(errnum, errbuf, sizeof(errbuf));
        fprintf(stderr, "Could not copy codec context: %s\n", errbuf);
        avcodec_free_context(&pCodecCtxOrig);
        avformat_close_input(&pFormatCtx);
        return -1;
    }

    // Find the decoder for the video stream
    pCodec = const_cast<AVCodec*>(avcodec_find_decoder(pCodecCtxOrig->codec_id));
    if (pCodec == NULL)
    {
        fprintf(stderr, "Unsupported codec!\n");
        avcodec_free_context(&pCodecCtxOrig);
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
    numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);
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
    av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height, 1);

    // 初始化 SDL2
    if (!initSDL(&window, &renderer, pCodecCtx->width, pCodecCtx->height)) {
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

    // initialize SWS context for software scaling
    sws_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
                             pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BILINEAR,
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

    // Read frames and save first five frames to disk
    int i = 0;
    while (av_read_frame(pFormatCtx, &packet) >= 0)
    {
        if (packet.stream_index == videoStream)
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
