#include "SDLShowVideo.h"

SDLShowVideo::SDLShowVideo()
    : texture_(NULL), renderer_(NULL), texture_(NULL), video_file(NULL), video_format_ctx(NULL)
{
}

SDLShowVideo::SDLShowVideo(char *filename)
{
}

SDLShowVideo::~SDLShowVideo()
{
    if (video_format_ctx)
    {
        avformat_close_input(&video_format_ctx);
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

bool SDLShowVideo::ReadVideoFile(char *filename)
{
    int errnum;
    char errbuf[128];
    bool flag = false;
    do
    {
        if ((errnum = avformat_open_input(&video_format_ctx, filename, NULL, NULL)) != 0)
        {
            av_strerror(errnum, errbuf, sizeof(errbuf));
            fprintf(stderr, "Could not open file %s: %s\n", filename, errbuf);
            break;
        }

        // Retrieve stream information
        if ((errnum = avformat_find_stream_info(video_format_ctx, NULL)) < 0)
        {
            av_strerror(errnum, errbuf, sizeof(errbuf));
            fprintf(stderr, "Could not find stream information: %s\n", errbuf);
            break;
        }

        flag = true;

    } while (0);

    return flag;
}

// 初始化 SDL2
bool initSDL(int width, int height)
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
