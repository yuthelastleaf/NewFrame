#include "SDLShowVideo.h"

#include <windows.h>


int main(int argc, char *argv[]) {

    if (argc < 2)
    {
        fprintf(stderr, "Please provide a movie file\n");
        return -1;
    }

    SDLShowVideo ssv(argv[1]);
    ssv.Start();

    while (true) {
        Sleep(10000);
    }

    return 0;
}


