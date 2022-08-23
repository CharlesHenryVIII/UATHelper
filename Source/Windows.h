#pragma once
#include "Threading.h"
#include "SDL.h"
#include "imgui.h"

#include <string>

std::string ToString(const char* fmt, ...);
void        RunProcess(const char* applicationPath, const char* arguments);
void        InitOS(SDL_Window* window);

static bool keepOpen = true;
void ShowErrorWindow(const std::string& title, const std::string& text);


struct StartProcessJob : Job
{
    std::string applicationPath;
    std::string arguments;
    virtual void RunJob() override;
};
