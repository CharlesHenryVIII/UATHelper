#include "Windows.h"
#include "Math.h"
#include "Windows/resource.h"

#include "SDL_syswm.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#include <combaseapi.h>

std::string ToString(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char buffer[4096];
    s32 i = vsnprintf(buffer, arrsize(buffer), fmt, args);
    va_end(args);
    return buffer;
}

s32 RunProcess(const char* path, const char* args, bool async)
{
    //TODO: Allow this to work for ASCII AND Unicode
    SHELLEXECUTEINFO info = {};
    info.cbSize = sizeof(SHELLEXECUTEINFO);
    info.fMask = SEE_MASK_NOASYNC | SEE_MASK_NOCLOSEPROCESS;
    info.hwnd;
    //info.lpVerb = "open";
    info.lpVerb = NULL;
    info.lpFile = path ? path : "cmd.exe";
    info.lpParameters = args;
    info.lpDirectory = NULL;
    info.nShow = SW_SHOW;
    info.hInstApp = NULL; //out
    info.lpIDList;
    info.lpClass;
    info.hkeyClass;
    info.dwHotKey;
    info.hProcess; //out

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    DEFER { CloseHandle(info.hProcess); };
    if (!ShellExecuteEx(&info))
    {
        std::string errorBoxTitle = ToString("ShellExecuteEx Error: %i", GetLastError());
        std::string errorText     = ToString("Application Path: %s\n"
                                             "Command Line Params: %s", info.lpFile, args);
        ShowErrorWindow(errorBoxTitle, errorText);
        assert(false);
        return 2;
    }
    if (!async)
    {
        DWORD result = WaitForSingleObject(info.hProcess, INFINITE);
        if (result)
        {
            std::string errorBoxTitle = ToString("WaitForSingleObject Error: %i", GetLastError());
            std::string errorText = ToString("Application Path: %s\n"
                "Command Line Params: %s", info.lpFile, args);
            ShowErrorWindow(errorBoxTitle, errorText);
            assert(false);
            return -1;
        }
        DWORD exitCode = {};
        if (!GetExitCodeProcess(info.hProcess, &exitCode))
        {
            std::string errorBoxTitle = ToString("GetExitCodeProcess Error: %i", GetLastError());
            std::string errorText = ToString("Application Path: %s\n"
                "Command Line Params: %s", info.lpFile, args);
            ShowErrorWindow(errorBoxTitle, errorText);
            return -1;
        }
        if (exitCode)
        {
            std::string errorBoxTitle = ToString("Program Exited with Code: %i", exitCode);
            std::string errorText = ToString("Application Path: %s\n"
                "Command Line Params: %s", info.lpFile, args);
            return ShowCustomErrorWindow(errorBoxTitle, errorText);
        }
    }
    return 0;
}

void StartProcessJob::RunJob()
{
    const char* path = applicationPath.size()   ? applicationPath.c_str()   : nullptr;
    const char* args = arguments.size()         ? arguments.c_str()         : nullptr;
    s32 result = RunProcess(path, args);
    if (result)
    {
        Threading::GetInstance().ClearJobs();
    }
}

void RunUATJob::RunJob()
{
    const char* path = applicationPath.size()   ? applicationPath.c_str()   : nullptr;
    const char* args = arguments.size()         ? arguments.c_str()         : nullptr;
    s32 result = RunProcess(path, args);
    if (result)
    {
        Threading::GetInstance().ClearJobs();
        switch (result)
        {
        case MessageBoxResponse_OpenLog:
        {
            std::string logLoc = rootPath + "Engine/Programs/AutomationTool/Saved/Logs/Log.txt";
            RunProcess(logLoc.c_str(), nullptr, true);
            break;
        }
        case -1:
        {
            //TODO: Handle the error case of GetExitCodeProcess failing, WaitforSingleObject, 
        }
        }
    }
}

HICON icon;
HMODULE instMod;
HWND windowHandle;

void InitOS(SDL_Window* window)
{
    instMod = GetModuleHandle(NULL);
    assert(instMod != NULL);
    SDL_SysWMinfo wminfo = {};
    SDL_VERSION(&wminfo.version);
    if (SDL_GetWindowWMInfo(window, &wminfo))
        windowHandle = wminfo.info.win.window;
    assert(windowHandle);
    icon = LoadIcon(instMod, MAKEINTRESOURCE(IDI_ICON1));
    assert(icon != NULL);
}

s32 ShowCustomErrorWindow(const std::string& title, const std::string& text)
{
    const SDL_MessageBoxButtonData buttons[] = {
        { 0,                                        MessageBoxResponse_Quit, "Quit Program" },
        { SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT,  MessageBoxResponse_Continue, "Continue" },
        { SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT,  MessageBoxResponse_OpenLog, "Open Log" },
    };
    const SDL_MessageBoxColorScheme colorScheme = {
        { /* .colors (.r, .g, .b) */
            /* [SDL_MESSAGEBOX_COLOR_BACKGROUND] */
            { 255,   0,   0 },
            /* [SDL_MESSAGEBOX_COLOR_TEXT] */
            {   0, 255,   0 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_BORDER] */
            { 255, 255,   0 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_BACKGROUND] */
            {   0,   0, 255 },
            /* [SDL_MESSAGEBOX_COLOR_BUTTON_SELECTED] */
            { 255,   0, 255 }
        }
    };
    const SDL_MessageBoxData messageboxdata = {
        //SDL_MESSAGEBOX_INFORMATION, /* .flags */
        //SDL_MESSAGEBOX_ERROR,
        SDL_MESSAGEBOX_WARNING,
        NULL, /* .window */
        title.c_str(), /* .title */
        nullptr,//text.c_str(), /* .message */
        SDL_arraysize(buttons), /* .numbuttons */
        buttons, /* .buttons */
        &colorScheme /* .colorScheme */
    };
    s32 buttonID = -1;
    if (SDL_ShowMessageBox(&messageboxdata, &buttonID) < 0) {
        SDL_Log("error displaying message box");
        //Quit Program
        SDL_Event e;
        e.type = SDL_QUIT;
        e.quit.timestamp = 0;
        SDL_PushEvent(&e);
        return 0;
    }
    //TODO: Add better error handling for this?
    assert(buttonID >= 0);

    if (buttonID == MessageBoxResponse_Quit)
    {
        SDL_Event e;
        e.type = SDL_QUIT;
        e.quit.timestamp = 0;
        SDL_PushEvent(&e);
    }
    return buttonID;
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

void NotifyWindowBuildFinished()
{
    FLASHWINFO info = {};
    info.hwnd = windowHandle;
    info.dwFlags = FLASHW_TRAY | FLASHW_TIMERNOFG;
    info.uCount;
    info.dwTimeout;
    info.cbSize = sizeof(info);

    FlashWindowEx(&info);
}

void ScanDirectoryForFileNames(const std::string& dir, std::vector<std::string>& out)
{
    out.clear();

    std::string d = dir;
    if (dir.size() < 2)
    {
        d = "*";
    }
    else
    {

        if (d[d.size() - 1] != '*')
        {
            if (d[d.size() - 1] != '/')
            {
                d += '/';
            }
            d += '*';
        }
    }
	

    WIN32_FIND_DATAA find_data;
    HANDLE handle = FindFirstFileA(d.c_str(), &find_data);
    while (true)
	{
		if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
            out.push_back(find_data.cFileName);
		}
        if (FindNextFileA(handle, &find_data) == 0)
        {
            //if (GetLastError() == ERROR_NO_MORE_FILES)
            break;
        }
	}
}

#include "shlobj_core.h"

static int CALLBACK BrowseFolderCallback(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
    if (uMsg == BFFM_INITIALIZED) {
        LPCTSTR path = LPCTSTR(lpData);
        ::SendMessage(hwnd, BFFM_SETSELECTION, true, (LPARAM)path);
    }
    return 0;
}

bool GetDirectoryFromUser(const std::string& currentDir, std::string& dir)
{
    std::string baseDir = currentDir;
    if (currentDir.size() == 0)
    {
        TCHAR buf[MAX_PATH] = { 0 };
        GetModuleFileName(NULL, buf, MAX_PATH);
        std::string::size_type pos = std::string(buf).find_last_of("\\/");
        baseDir = std::string(buf).substr(0, pos);
    }
    dir.clear();
    dir.resize(MAX_PATH);
    int imageIndex = 0;
    BROWSEINFOA info = {
        .hwndOwner = windowHandle,
        .pidlRoot = NULL,
        .pszDisplayName = NULL,//dir.data(),
        .lpszTitle = "Select Config Directory",
        .ulFlags =  BIF_USENEWUI, //BIF_EDITBOX | BIF_NEWDIALOGSTYLE,
        .lpfn = BrowseFolderCallback,//NULL,
        .lParam = (LPARAM)baseDir.c_str(), //NULL,
        .iImage = imageIndex,
    };
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    PIDLIST_ABSOLUTE pidl = SHBrowseForFolderA(&info);
    if (pidl == NULL)
        return false;
    BOOL result = SHGetPathFromIDListA(pidl, dir.data());

    auto pos = dir.find_first_of('\0');
    if (pos != std::string::npos)
        dir.resize(pos);

    if (result)
    {
        std::string_view dir1 = dir;
        std::string_view dir2 = baseDir;
        if (dir1.find(dir2) != std::string::npos && dir2.find(dir1) != std::string::npos)
        {
            dir.clear();
        }
        return true;
    }
    return false;
}
