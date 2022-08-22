#pragma once
#include "Threading.h"
#include "SDL.h"

#include <string>

std::string ToString(const char* fmt, ...);
void        RunProcess(const char* applicationPath, const char* arguments);
int         ShowErrorWindow(const std::string& title, const std::string& text);
void        InitOS(SDL_Window* window);

struct StartProcessJob : Job
{
    std::string applicationPath;
    std::string arguments;
    virtual void RunJob() override;
};
