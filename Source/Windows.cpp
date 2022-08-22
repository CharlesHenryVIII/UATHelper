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

int ShowErrorWindow(const std::string& title, const std::string& text)
{
    return SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), text.c_str(), NULL);
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
    if (instMod == NULL)
    {
        DWORD err = GetLastError();
        ShowErrorWindow("Error GetModuleHandle()", ToString("GetModuleHandle() Returned: %i", err));
        return;
    }
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    if (!SDL_GetWindowWMInfo(window, &wminfo))
    {
        const char* err = SDL_GetError();
        ShowErrorWindow("Error SDL_GetWindowWMInfo()", ToString("SDL_GetWindowWMInfo() Returned: %s", err));
        return;
    }
    HWND hwnd = wminfo.info.win.window;

    icon = LoadIcon(instMod, MAKEINTRESOURCE(IDI_ICON1));
    if (icon == NULL)
    {
        DWORD err = GetLastError();
        ShowErrorWindow("Error LoadIcon()", ToString("LoadIcon() Returned: %i", err));
        return;
    }
}
