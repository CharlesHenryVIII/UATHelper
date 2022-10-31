#pragma once
#include "Threading.h"
#include "SDL.h"
#include "imgui.h"

#include <string>

std::string ToString(const char* fmt, ...);
s32         RunProcess(const char* path, const char* args = nullptr, bool async = false);
void        InitOS(SDL_Window* window);

static bool keepOpen = true;
void ShowErrorWindow        (const std::string& title, const std::string& text);
s32 ShowCustomErrorWindow  (const std::string& title, const std::string& text);
void NotifyWindowBuildFinished();
void ScanDirectoryForFileNames(const std::string& dir, std::vector<std::string>& out);
bool GetDirectoryFromUser(const std::string& currentDir, std::string& dir);
enum MessageBoxResponse : s32 {
    MessageBoxResponse_Invalid,
    MessageBoxResponse_OpenLog,
    MessageBoxResponse_Continue,
    MessageBoxResponse_Quit,
    MessageBoxResponse_Count,
};

struct StartProcessJob : Job
{
    std::string applicationPath;
    std::string arguments;
    virtual void RunJob() override;
};

struct RunUATJob : Job
{
    std::string applicationPath;
    std::string arguments;
    std::string rootPath;
    virtual void RunJob() override;
};
