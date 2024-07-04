#include "SDLShowVideo.h"

SDLShowVideo::SDLShowVideo()
    : texture_(NULL)
    , window_(NULL)
    , renderer_(NULL)
    , video_file_(NULL)
    , video_format_ctx_(NULL)
{
}

SDLShowVideo::SDLShowVideo(char *filename)
    : texture_(NULL)
    , window_(NULL)
    , renderer_(NULL)
    , video_file_(NULL)
    , video_format_ctx_(NULL)
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
    
    
}
