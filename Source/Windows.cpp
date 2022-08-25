#include "Windows.h"
#include "Math.h"
#include "Windows/resource.h"

#include "SDL_syswm.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>

std::string ToString(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buffer[4096];
    s32 i = vsnprintf(buffer, arrsize(buffer), fmt, args);
    va_end(args);
    return buffer;
}

void RunProcess(const char* applicationPath, const char* arguments)
{
    LPSTR lpApplicationPath = const_cast<char*>(applicationPath);
    LPSTR lpCommandLine = const_cast<char*>(arguments);

    DWORD newProcessFlags = CREATE_NEW_CONSOLE | NORMAL_PRIORITY_CLASS;

    STARTUPINFOA startupInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    startupInfo.dwFlags = STARTF_USESHOWWINDOW;
    startupInfo.wShowWindow = SW_SHOWDEFAULT; //SW_MINIMIZE
    startupInfo.cb = sizeof(startupInfo);

    _PROCESS_INFORMATION pi = {};
    DEFER{
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    };
    //TODO: Allow this to work for ASCII AND Unicode
    if (!CreateProcess(
        lpApplicationPath,  //LPCSTR                  lpApplicationName,
        lpCommandLine,      //LPSTR                   lpCommandLine,
        NULL,               //LPSECURITY_ATTRIBUTES   lpProcessAttributes,
        NULL,               //LPSECURITY_ATTRIBUTES   lpThreadAttributes,
        true,               //BOOL                    bInheritHandles,
        newProcessFlags,    //DWORD                   dwCreationFlags,
        NULL,               //LPVOID                  lpEnvironment,
        NULL,               //LPCSTR                  lpCurrentDirectory,
        &startupInfo,       //LPSTARTUPINFOA          lpStartupInfo,
        &pi                 //LPPROCESS_INFORMATION   lpProcessInformation
    ))
    {
        
        std::string errorBoxTitle = ToString("Create Process Error: %i", GetLastError());
        std::string errorText     = ToString("Application Path: %s\n"
                                             "Command Line Params: %s", applicationPath, arguments);
        ShowErrorWindow(errorBoxTitle, errorText);
        return;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
}

void StartProcessJob::RunJob()
{
    if (arguments.size())
        RunProcess(applicationPath.c_str(), arguments.c_str());
    else
        RunProcess(applicationPath.c_str(), nullptr);
}

HICON icon;
HMODULE instMod;

void InitOS(SDL_Window* window)
{
    instMod = GetModuleHandle(NULL);
    assert(instMod != NULL);
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    assert(SDL_GetWindowWMInfo(window, &wminfo));
    icon = LoadIcon(instMod, MAKEINTRESOURCE(IDI_ICON1));
    assert(icon != NULL);
}


void ShowErrorWindow(const std::string& title, const std::string& text)
{
#if 1
    int msgboxID = MessageBox(
        NULL,
        text.c_str(),
        title.c_str(),
        MB_ABORTRETRYIGNORE | MB_ICONSTOP | MB_DEFBUTTON1 | MB_APPLMODAL
    );

    switch (msgboxID)
    {
    case IDABORT:
        SDL_Event e;
        e.type = SDL_QUIT;
        e.quit.timestamp = 0;
        SDL_PushEvent(&e);
        break;
    case IDRETRY:
        break;
    case IDIGNORE:
        break;
    }
#else
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoSavedSettings;
    const ImVec2 min = { 260, 100 };
    const ImVec2 windowSize = ImGui::GetMainViewport()->WorkSize;
    const ImVec2 max = {windowSize.x - 200, windowSize.y - 200};
    ImGui::SetNextWindowSizeConstraints(min, max);
    ImGui::SetNextWindowPos(ImVec2(windowSize.x / 2, windowSize.y / 2), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::OpenPopup(title.c_str());
    if (ImGui::BeginPopupModal(title.c_str(), NULL, flags))
    {
        ImGui::TextWrapped(text.c_str());
        if (ImGui::Button("Continue"))
            ImGui::CloseCurrentPopup();
        ImGui::SameLine();
        if (ImGui::Button("Copy to Clipboard"))
            SDL_SetClipboardText(text.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Exit"))
        {
            SDL_Event e;
            e.type = SDL_QUIT;
            e.quit.timestamp = 0;
            SDL_PushEvent(&e);
        }
        ImGui::EndPopup();
    }
#endif
}
