#include <iostream>
#include "handle_param.h"
#include "FFCmd.h"

FFCmd* g_ffcmd = nullptr;

void handle_param_d(const char* arg) {
    std::cout << "Handling -d with argument: " << arg << std::endl;
}

void handle_param_f(const char* arg) {
    std::cout << "Handling -f with argument: " << arg << std::endl;
}

void handle_param_i(const char *arg)
{
    if(!g_ffcmd) {
        g_ffcmd = new FFCmd(arg);
        g_ffcmd->av_show_info();
    }
}
