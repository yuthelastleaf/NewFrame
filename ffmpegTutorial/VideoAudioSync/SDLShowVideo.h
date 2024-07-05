#pragma once

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

#include <memory>

#include "stream_parser.h"

class SDLShowVideo
{
private:
    /* data */
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    SDL_Texture* texture_;

    char* video_file_;
    bool show_start_;
    AVFormatContext *video_format_ctx_;

    std::unique_ptr<stream_parser> video_streams_;
    std::unique_ptr<stream_parser> audio_streams_;

public:
    SDLShowVideo(/* args */);
    SDLShowVideo(char* filename);
    ~SDLShowVideo();

    bool InitSDL();
    bool ReadVideoFile(char* filename);

    void ShowVideo();
    
    void Start();
    void Pause();
    void Stop();

};

