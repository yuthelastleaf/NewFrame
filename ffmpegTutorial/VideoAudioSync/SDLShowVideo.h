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

class SDLShowVideo
{
private:
    /* data */
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    SDL_Texture* texture_;

    char* video_file_;
    AVFormatContext *video_format_ctx;

public:
    SDLShowVideo(/* args */);
    SDLShowVideo(char* filename);
    ~SDLShowVideo();

    bool ReadVideoFile(char* filename);
    
    void Start();
    void Pause();
    void Stop();

};

