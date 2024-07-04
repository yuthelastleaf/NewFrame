#include "SDLShowVideo.h"


int main(int argc, char *argv[]) {

    if (argc < 2)
    {
        fprintf(stderr, "Please provide a movie file\n");
        return -1;
    }

    SDLShowVideo ssv(argv[1]);

    return 0;
}


