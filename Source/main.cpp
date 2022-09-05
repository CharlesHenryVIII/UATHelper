// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "Tracy.hpp"

#include "Windows.h"
#include "Math.h"
#include "Threading.h"
#include "Config.h"
#include "Themes.h"

#include <stdio.h>
#include <string>
#include <vector>

#include <SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif



[[nodiscard]] inline ImVec2 HadamardProduct(const ImVec2& a, const ImVec2& b)
{
    return { a.x * b.x, a.y * b.y };
}

bool FindNumberInVector(const std::vector<s32>& data, const s32 val)
{
    for (s32 i = 0; i < data.size(); i++)
    {
        if (data[i] == val)
            return true;
    }
    return false;
}

void RemoveNumberInVector(std::vector<s32>& data, const s32 val)
{
    std::erase_if(data,
        [val](s32 a)
        {
            return a == val;
        });
}

void TextCentered(std::string text) 
{
    float win_width = ImGui::GetWindowSize().x;
    float text_width = ImGui::CalcTextSize(text.c_str()).x;

    // calculate the indentation that centers the text on one line, relative
    // to window left, regardless of the `ImGuiStyleVar_WindowPadding` value
    float text_indentation = (win_width - text_width) * 0.5f;

    // if text is too long to be drawn on one line, `text_indentation` can
    // become too small or even negative, so we check a minimum indentation
    float min_indentation = 20.0f;
    if (text_indentation <= min_indentation) {
        text_indentation = min_indentation;
    }

    ImGui::SameLine(text_indentation);
    ImGui::PushTextWrapPos(win_width - text_indentation);
    ImGui::Text(text.c_str());
    ImGui::PopTextWrapPos();
}



int DynamicTextCallback(ImGuiInputTextCallbackData* data)
{
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        assert(data->UserData);
        if (!data->UserData)
            return 1;
        std::string* string = (std::string*)data->UserData;
        //assert((char*)string->data() == (char*)data->Buf);
#if 1
            string->resize(data->BufTextLen);
            data->Buf = string->data();
#else
        if (data->BufSize < data->BufTextLen)
        {
            string->resize(data->BufSize);
            data->Buf = string->data();
        }
#endif
    }
    return 0;
}
//NOTE(CSH): Ideally there would be a function overload that takes a const char* for the title input but the function cannot be determined
//since passsing a string could convert to a const char* or be built into a std::string
bool InputTextDynamicSize(const std::string& title, std::string& s, ImGuiInputTextFlags flags = ImGuiInputTextFlags_None)
{
    return ImGui::InputText(title.c_str(), s.data(), s.capacity(), flags | ImGuiInputTextFlags_CallbackResize, DynamicTextCallback, &s);
}
bool InputTextMultilineDynamicSize(const std::string& title, std::string& s, ImGuiInputTextFlags flags = ImGuiInputTextFlags_None)
{
    return ImGui::InputTextMultiline(title.c_str(), const_cast<char*>(title.data()), s.capacity(), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 2), flags | ImGuiInputTextFlags_CallbackResize, DynamicTextCallback, &s);
}


bool NameStatusButtonAdd(const std::string& buttonName, std::string& text, int codeLine)
{
    ImGui::SameLine();
    std::string id = "##" + std::to_string(codeLine);
    float buttonSize = 25.0f;
#if 1
    float inputSize = 100.0f;
    float availableWidth = ImGui::GetContentRegionAvail().x;
    if (availableWidth < inputSize)
    {
        ImGui::NewLine();
    }
#else
    float inputSize = 20.0f;
    //Scaling input size based on region left
    float availableWidth = ImGui::GetContentRegionAvail().x;
    if (availableWidth < inputSize)
    {
        ImGui::NewLine();
    }
    availableWidth = ImGui::GetContentRegionAvail().x;
    inputSize = availableWidth * 0.25f
#endif
    ImGui::SetNextItemWidth(inputSize - buttonSize);
    InputTextDynamicSize(id, text);
    ImGui::SameLine();
    std::string fullButtonName = "Add " + buttonName;
    return ImGui::Button(fullButtonName.c_str()) && text.size();
}

void RemoveStartAndEndSpaces(std::string& s)
{
    while (s[0] == ' ')
    {
        s.erase(0, 1);
    }
    while (s[s.size() - 1] == ' ')
    {
        s.erase(s.size() - 1, 1);
    }
}

void ExecutionSection(const std::string& sectionTitle, std::vector<std::string>& names, std::vector<s32>& enables, std::string& inputString)
{
    std::string sectionTitleEvents = sectionTitle + " Events:";
    //ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
    ImGui::SetNextItemOpen(true, ImGuiCond_Once);
    if (ImGui::TreeNode(sectionTitleEvents.c_str()))
    {
        DEFER{ ImGui::TreePop(); };
        bool inputSuccess = false;
        std::string textInputTitle = "##" + sectionTitle + " Event";
        inputSuccess |= InputTextDynamicSize(textInputTitle.c_str(), inputString, ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();
        inputSuccess |= ImGui::Button("Add");
        if (inputSuccess)
        {
            RemoveStartAndEndSpaces(inputString);
            names.push_back({ inputString });
            inputString.clear();
        }
        if (names.size() == 0)
            return;


        ImGuiTableFlags tableFlags =
            ImGuiTableFlags_RowBg |
            ImGuiTableFlags_BordersV |
            ImGuiTableFlags_BordersOuterV |
            ImGuiTableFlags_BordersInnerV |
            ImGuiTableFlags_BordersH |
            ImGuiTableFlags_BordersOuterH |
            ImGuiTableFlags_BordersInnerH |
            ImGuiTableFlags_ScrollX |
            ImGuiTableFlags_ScrollY;
            //ImGuiTableFlags_NoSavedSettings;

        //NOTE(CSH): 1 larger than the array to have size leftover for the horizontal scroll bar
        //capping the height at 5 so you can see the top row while scrolling horizontally
        //TODO: change the max height to varry with the size of the child window
        const int maxTableHeight = 5;
        ImVec2 tableSize = ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * (Min(maxTableHeight, (int)names.size()) + 1)); 
        const int tableColumnCount = 2;
        if (ImGui::BeginTable("table_advanced", tableColumnCount, tableFlags, tableSize, 0.0f))
        {
            DEFER{ ImGui::EndTable(); };

            const ImGuiTableColumnFlags columnFlags = ImGuiTableColumnFlags_WidthFixed;
            assert(names.size());
            float longestText = 0;
            for (const auto& item : names)
            {
                ImVec2 textSize = ImGui::CalcTextSize(item.c_str());
                longestText = Max(longestText, textSize.x);
            }
            ImGui::TableSetupScrollFreeze(1, 0);
            ImGui::TableSetupColumn("Status",   columnFlags | ImGuiTableColumnFlags_NoHide);
            ImGui::TableSetupColumn("String",   columnFlags, longestText);

            //ImGui::PushButtonRepeat(true);
            {
                for (int row_n = 0; row_n < names.size(); row_n++)
                {
                    auto item = names[row_n];

                    ImGui::PushID(item.c_str());
                    DEFER{ ImGui::PopID(); };

                    ImGui::TableNextRow(ImGuiTableRowFlags_None, 0);
                    if (ImGui::TableSetColumnIndex(0))
                    {
                        std::string buttonLabel = "Disabled";
                        float color = 0.0f;
                        const bool enabled = FindNumberInVector(enables, row_n);
                        if (enabled)
                        {
                            color = 2.0f / 7.0f;
                            buttonLabel = "Enabled";
                        }
                        
                        ImGui::PushStyleColor(ImGuiCol_Button,          (ImVec4)ImColor::HSV(color, 0.6f, 0.6f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,   (ImVec4)ImColor::HSV(color, 0.7f, 0.7f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive,    (ImVec4)ImColor::HSV(color, 0.8f, 0.8f));
                        if (ImGui::SmallButton(buttonLabel.c_str()))
                        {
                            if (enabled)
                                RemoveNumberInVector(enables, row_n);
                            else
                                enables.push_back(row_n);
                        }
                        ImGui::PopStyleColor(3);
                    }


                    ImGui::TableSetColumnIndex(1);
                    ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
                    ImGui::Selectable(item.c_str(), false, selectable_flags, ImVec2(0, 0));
                    if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
                    {
                        int nextIndex = row_n + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
                        if (nextIndex >= 0 && nextIndex < names.size())
                        {
                            names[row_n] = names[nextIndex];
                            names[nextIndex] = item;
                            ImGui::ResetMouseDragDelta();
                        }
                    }
                }
            }
        }
    }
}

bool SeperatePathAndArguments(const std::string& input, std::string& path, std::string& args)
{
    char charSeperator = ' ';
    if (input[0] == '\"')
    {
        charSeperator = '\"';
    }
    //look for end quote or space
    s32 i = 1;
    for (; i < input.size(); i++)
    {
        if (input[i] == charSeperator)
            break;
    }
    s32 maxValue = Min<s32>((s32)input.size(), i + 1);
    path = input.substr(0,          maxValue);
    if (maxValue == input.size())
    {
        args.clear();
        return false;
    }

    args = input.substr(maxValue,   input.size() - (maxValue));
    return true;
}

void RunProcess(const std::string& name, Threading& thread)
{
    std::string path;
    std::string args;
    SeperatePathAndArguments(name, path, args);
    RemoveStartAndEndSpaces(path);
    if (args.size())
        RemoveStartAndEndSpaces(args);
    StartProcessJob* job = new StartProcessJob();
    job->applicationPath = path;
    job->arguments = args;
    thread.SubmitJob(job);
}

void RunProcessList(const std::vector<std::string>& names, const std::vector<s32>& enabledInts, Threading& thread)
{
    for (s32 i = 0; i < enabledInts.size(); i++)
    {
        RunProcess(names[enabledInts[i]], thread);
    }
}

void CleanPathString(std::string& s)
{
    size_t pos = s.find('\\');
    while (pos != std::string::npos)
    {
        s.replace(pos, 1, "/", 1);
        pos = s.find('\\');
    }
}

void HelpMarker(const std::string& desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool GetCStringFromPlatformSettings(void* data, int idx, const char** out_text)
{
    if (!data)
        return false;
    const std::vector<PlatformSettings>& d = *(std::vector<PlatformSettings>*)data;
    if (idx >= d.size())
        return false;
    *out_text = d[idx].name.c_str();
    return true;
}

bool GetCStringFromThemes(void* data, int idx, const char** out_text)
{
    if (!data)
        return false;
    const Theme* d = (Theme*)data;
    *out_text = d[idx].name;
    return true;
}


// Main code
int main(int, char**)
{
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_Rect screenSize = {};
    if (SDL_GetDisplayBounds(0, &screenSize))
    {
        SDL_Log("SDL_GetDisplayBounds failed: %s", SDL_GetError());
        return 1;
    }
    float normalRatio = 16.0f / 9.0f;
    float displayRatio = float(screenSize.w) / float(screenSize.h);
    int windowHeight = 0;
    int windowWidth = 0;
    if (displayRatio < normalRatio)
    {
        windowWidth  = int(float(screenSize.w) / 2.0f);
        windowHeight = int(float(screenSize.w) / normalRatio);
    }
    else
    {
        windowHeight = int(float(screenSize.h / 2));
        windowWidth  = int(float(normalRatio * windowHeight));
    }

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("UATHelper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    Threading& threading = Threading::GetInstance();
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = NULL;
    ImGuiStyle& style = ImGui::GetStyle();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    //ImGui::StyleColorsClassic();
    // Setup Dear ImGui style
    //ImGuiTheme_Basic(style);
    //ImGui::StyleColorsDark();
    //ImGuiColor_Grey();
    //ImGuiColor_WildCard();
    //ImGuiColor_GreenAccent();
    //ImGuiColor_RedAccent();
    //ImGuiColor_Grey();

    //ImGuiTheme_Basic();
    //ImGuiTheme_SimpleRounding();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
    InitOS(window);
    ThemesInit();
    
    Settings settings;
    if (!LoadConfig(settings))
    {
        LoadDefaults(settings);
    }

    //SaveConfig(settings);
    //return 0;
    std::string finalCommandLine;
    bool buildRunning = false;
    bool rebuildCommandline = true;

    //ImFont* mainFont = io.Fonts->AddFontFromFileTTF("Assets/DroidSans.ttf", 16);
    //io.Fonts->Build();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = false;
    bool exitProgram = false;
    bool modifiedSettings = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        {
            ZoneScopedN("Frame Update:");
            // Poll and handle events (inputs, window resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
            // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT)
                    exitProgram = true;
                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                    exitProgram = true;
                //done = true;
            }

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();
            //ImGui::PushFont(mainFont);
            if (!threading.GetJobsInFlight())
                buildRunning = false;

            if (!modifiedSettings)
                modifiedSettings = !ConfigIsSameAsLastLoad(settings);



            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Always, {});
            ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
            ImGui::SetNextWindowBgAlpha(1.0f); // Transparent background
            ImGuiWindowFlags windowFlags =
                //ImGuiWindowFlags_NoBackground |
#if 0
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
#else
                ImGuiWindowFlags_NoDecoration |
#endif
                ImGuiWindowFlags_MenuBar |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_NoNav |
                ImGuiWindowFlags_NoMove;
            if (modifiedSettings)
            {
                windowFlags |= ImGuiWindowFlags_UnsavedDocument;
            }

            if (ImGui::Begin("Main", nullptr, windowFlags))
            {
                ZoneScopedN("Main");
                if (ImGui::BeginMenuBar())
                {
                    if (ImGui::BeginMenu("Settings"))
                    {
                        //ZoneScopedN("Settings");
                        ImGui::Text("Color:");
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(100);
                        if (ImGui::Combo("##Color", &settings.colorSelection, GetCStringFromThemes, &ColorOptions, (s32)Color_Count))
                            Color_Set(settings.colorSelection);
                        ImGui::Text("Style:");
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(100);
                        if (ImGui::Combo("##Style", &settings.styleSelection, GetCStringFromThemes, &StyleOptions, (s32)Style_Count))
                            Style_Set(settings.styleSelection);

                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Config"))
                    {
                        ZoneScopedN("Config");
                        if (ImGui::MenuItem("Save Config", "Ctrl+S"))
                            SaveConfig(settings);
                        if (ImGui::MenuItem("Load Config", "Ctrl+L"))
                            LoadConfig(settings);
                        if (ImGui::MenuItem("Clear Config"))
                            ClearConfig(settings);
                        ImGui::EndMenu();
                    }
                    ImGui::EndMenuBar();
                }
                ImGuiWindowFlags sectionFlags =
                    ImGuiWindowFlags_NoResize |
                    ImGuiWindowFlags_NoSavedSettings |
                    ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoFocusOnAppearing |
                    ImGuiWindowFlags_NoMove;

                f32 xScale = 1.0f;
                float topWindowHeightScale = 0.12f;
                ImVec2 locationScale = { xScale / 2, topWindowHeightScale };
                ImVec2 locationSize = HadamardProduct(viewport->WorkSize, locationScale);
                locationSize.x -= 1.5f * style.WindowPadding.x;
                if (ImGui::BeginChild("File Paths", locationSize, true, sectionFlags | ImGuiWindowFlags_NoScrollbar))
                {
                    ZoneScopedN("File Paths");
#ifdef DEBUG
                    ImGui::Checkbox("Show Demo Window", &show_demo_window);
                    ImGui::SameLine();
#endif
                    TextCentered("File Paths");

                    ImGui::Text("Main Directory");
                    ImGui::SameLine();
                    std::string demoMainDir = "C:/UnrealEngine/Project";
                    HelpMarker(demoMainDir);
                    ImGui::SameLine();
                    ImGui::PushItemWidth(-FLT_MIN);
                    InputTextDynamicSize("##" + demoMainDir, settings.rootPath);
                    CleanPathString(settings.rootPath);

                    ImGui::Text("Path to .uproject");
                    std::string demoProjectPath = "C:/UnrealEngine/Project/Title/title.uproject";
                    ImGui::SameLine();
                    HelpMarker(demoProjectPath);
                    ImGui::SameLine();
                    InputTextDynamicSize("##" + demoProjectPath, settings.projectPath);
                    CleanPathString(settings.projectPath);
                }
                ImGui::EndChild();
                ImGui::SameLine();

                ImVec2 platformScale = { xScale / 2, topWindowHeightScale };
                ImVec2 platformSize = HadamardProduct(viewport->WorkSize, platformScale);
                platformSize.x -= 1.5f * style.WindowPadding.x;
                if (ImGui::BeginChild("Grouping", platformSize, true, sectionFlags))
                {
                    ZoneScopedN("Grouping");
                    ImGui::Text("Platform Selection");
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(150);
                    ImGui::Combo("##Platform Selection", &settings.platformSelection, GetCStringFromPlatformSettings,
                        &settings.platformOptions, (s32)settings.platformOptions.size());
                    static std::string platformAddText;
                    if (NameStatusButtonAdd("Platform", platformAddText, __LINE__))
                    {
                        settings.platformOptions.push_back({ platformAddText });
                        platformAddText.clear();
                    }

                    ImGui::Text("Version Selection");
                    ImGui::SameLine();
                    static std::string versionInputName;
                    if (NameStatusButtonAdd("Version", versionInputName, __LINE__))
                    {
                        settings.versionOptions.push_back({ versionInputName });
                        versionInputName.clear();
                    }
                    ImGui::NewLine();
                    float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
                    float last_button_x2 = 0;
                    ImGuiStyle& style = ImGui::GetStyle();
                    for (int i = 0; i < settings.versionOptions.size(); i++)
                    {
                        float button_szx = ImGui::CalcTextSize(settings.versionOptions[i].c_str()).x + 2 * style.FramePadding.x;
                        float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_szx;
                        if (next_button_x2 < window_visible_x2)
                            ImGui::SameLine(0, 1);
                        bool found = FindNumberInVector(settings.platformOptions[settings.platformSelection].enabledVersions, i);
                        bool checkbox = found;
                        ImGui::Checkbox(settings.versionOptions[i].c_str(), &checkbox);
                        if (checkbox != found)
                        {
                            if (found)
                                RemoveNumberInVector(settings.platformOptions[settings.platformSelection].enabledVersions, i);
                            else
                                settings.platformOptions[settings.platformSelection].enabledVersions.push_back(i);
                        }
                        last_button_x2 = ImGui::GetItemRectMax().x;
                    }


                }
                ImGui::EndChild();

                ImVec2 switchesScale = { 0, 0.2f };
                ImVec2 switchesSize = HadamardProduct(viewport->WorkSize, switchesScale);
                if (ImGui::BeginChild("Switches", switchesSize, true, sectionFlags))
                {
                    ZoneScopedN("Switches");
                    TextCentered("Switch Selection");
                    ImGui::NewLine();
                    float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
                    float last_button_x2 = 0;
                    ImGuiStyle& style = ImGui::GetStyle();
                    for (int i = 0; i < settings.switchOptions.size(); i++)
                    {
                        float button_szx = ImGui::CalcTextSize(settings.switchOptions[i].c_str()).x + 2 * style.FramePadding.x;
                        float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_szx;
                        if (next_button_x2 < window_visible_x2)
                            ImGui::SameLine(0, 1);
                        bool found = FindNumberInVector(settings.platformOptions[settings.platformSelection].enabledSwitches, i);
                        bool checkbox = found;
                        ImGui::Checkbox(settings.switchOptions[i].c_str(), &checkbox);
                        if (checkbox != found)
                        {
                            if (found)
                                RemoveNumberInVector(settings.platformOptions[settings.platformSelection].enabledSwitches, i);
                            else
                                settings.platformOptions[settings.platformSelection].enabledSwitches.push_back(i);
                        }
                        last_button_x2 = ImGui::GetItemRectMax().x;
                    }
                    static std::string name;
                    if (NameStatusButtonAdd("Switch", name, __LINE__))
                    {
                        settings.switchOptions.push_back({ name });
                        name.clear();
                    }
                }
                ImGui::EndChild();

                ImVec2 executionScale = { 0, 0.35f };
                ImVec2 executionSize = HadamardProduct(viewport->WorkSize, executionScale);
                if (ImGui::BeginChild("Building", executionSize, true, sectionFlags))
                {
                    //ZoneScopedN("Building");
                    TextCentered("Build Events");

                    static std::string preBuildInput;
                    if (settings.platformOptions.size())
                        ExecutionSection("Pre-Build", settings.preBuildEvents, settings.platformOptions[settings.platformSelection].enabledPreBuild, preBuildInput);

                    static std::string postBuildInput;
                    if (settings.platformOptions.size())
                        ExecutionSection("Post-Build", settings.postBuildEvents, settings.platformOptions[settings.platformSelection].enabledPostBuild, postBuildInput);
                }
                ImGui::EndChild();

                ImVec2 commandScale = { 0, 0.25f };
                ImVec2 commandSize = HadamardProduct(viewport->WorkSize, commandScale);
                if (ImGui::BeginChild("Command Line", ImVec2(0, 0), true, sectionFlags))
                {
                    ZoneScopedN("Command Line");
                    TextCentered("Command Line Output");

                    bool invalid_projectPath = settings.projectPath.size() < 10;
                    bool invalid_rootPath = settings.rootPath.size() < 3;
                    bool invalid_platformOptions = !(settings.platformOptions.size());
                    bool invalid_versionSelected = true;
                    if (!invalid_platformOptions)
                        invalid_versionSelected = !(settings.platformOptions[settings.platformSelection].enabledVersions.size());
                    bool commandLineInvalid = invalid_projectPath || invalid_rootPath || invalid_platformOptions || invalid_versionSelected;

                    finalCommandLine.clear();
                    if (commandLineInvalid)
                    {
                        if (invalid_rootPath)
                        {
                            finalCommandLine = "Invalid Main Directory";
                        }
                        else if (invalid_projectPath)
                        {
                            finalCommandLine = "Invalid Project Path";
                        }
                        else if (invalid_platformOptions)
                        {
                            finalCommandLine = "Invalid Platform Options";
                        }
                        else if (invalid_versionSelected)
                        {
                            finalCommandLine = "Invalid Version Selected";
                        }
                    }
                    else
                    {
                        finalCommandLine += settings.rootPath.c_str();
                        finalCommandLine += "Engine/Build/BatchFiles/RunUAT.bat BuildCookRun ";
                        finalCommandLine += "-project=";
                        finalCommandLine += settings.projectPath.c_str();
                        finalCommandLine += " -targetplatform=";
                        finalCommandLine += settings.platformOptions[settings.platformSelection].name;
                        finalCommandLine += " -clientconfig=";
                        bool alreadyOneEnabled = false;
                        for (const auto& optionIndex : settings.platformOptions[settings.platformSelection].enabledVersions)
                        {
                            if (alreadyOneEnabled)
                                finalCommandLine += "+";
                            finalCommandLine += settings.versionOptions[optionIndex];
                            alreadyOneEnabled = true;
                        }
                        finalCommandLine += " -servertargetplatform=win64";
                        finalCommandLine += " -serverconfig=Development";

                        for (const auto& optionIndex : settings.platformOptions[settings.platformSelection].enabledSwitches)
                        {
                            finalCommandLine += " -";
                            finalCommandLine += settings.switchOptions[optionIndex];
                        }
                    }

                    ImGui::TextWrapped(finalCommandLine.c_str());

                    if (ImGui::Button("Copy To Clipboard"))
                    {
                        SDL_SetClipboardText(finalCommandLine.c_str());
                    }
                    ImGui::SameLine();
                    if (buildRunning || commandLineInvalid)
                        ImGui::BeginDisabled();
                    if (ImGui::Button("RUN"))
                    {
                        RunProcessList(settings.preBuildEvents,
                            settings.platformOptions[settings.platformSelection].enabledPreBuild,
                            threading);
#if NDEBUG
                        RunProcess(finalCommandLine, threading);
#endif
                        RunProcessList(settings.postBuildEvents,
                            settings.platformOptions[settings.platformSelection].enabledPostBuild,
                            threading);
                    }
                    if (buildRunning || commandLineInvalid)
                        ImGui::EndDisabled();
                    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
                }
                ImGui::EndChild();


                if (exitProgram)
                {
                    if (modifiedSettings)
                    {
                        ImGuiWindowFlags flags =
                            ImGuiWindowFlags_NoCollapse |
                            ImGuiWindowFlags_NoSavedSettings;
                        const ImVec2 min = { 260, 100 };
                        const ImVec2 windowSize = ImGui::GetMainViewport()->Size;
                        const ImVec2 max = { windowSize.x - 200, windowSize.y - 200 };
                        ImGui::SetNextWindowSizeConstraints(min, max);
                        ImGui::SetNextWindowPos(ImVec2(windowSize.x / 2, windowSize.y / 2), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                        const char* popupName = "Unsaved Changes";
                        ImGui::OpenPopup(popupName, ImGuiPopupFlags_None);
                        if (ImGui::BeginPopupModal(popupName, &exitProgram, flags))
                        {
                            float buttonHeight = 30.0f;
                            ImGui::TextWrapped("The application is being closed without being saved, are you sure you want to continue?");
                            if (ImGui::Button("Save and Exit", ImVec2(-FLT_MIN, buttonHeight)))
                            {
                                SaveConfig(settings);
                                done = true;
                            }
                            ImVec2 saveButtonSize = ImGui::GetItemRectSize();
                            if (ImGui::Button("Exit Without Saving", ImVec2(saveButtonSize.x * (2.0f / 3.0f), buttonHeight)))
                            {
                                done = true;
                            }
                            ImGui::SameLine();
                            if (ImGui::Button("Cancel", ImVec2(-FLT_MIN, buttonHeight)))
                            {
                                ImGui::CloseCurrentPopup();
                                exitProgram = false;
                            }
                            ImGui::EndPopup();
                        }
                    }
                    else
                    {
                        done = true;
                    }
                }

                ImGui::End();
            }


            if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);

            {
                ZoneScopedN("ImGui Render");
                // Rendering
                //ImGui::PopFont();
                {
                    ZoneScopedN("ImGui Render");
                    ImGui::Render();
                }
                {
                    ZoneScopedN("glViewport");
                    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
                }
                {
                    ZoneScopedN("glClearColor");
                    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
                }
                {
                    ZoneScopedN("glClear");
                    glClear(GL_COLOR_BUFFER_BIT);
                }
                {
                    ZoneScopedN("RenderDrawData");
                    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
                }
            }
        }
        {
            ZoneScopedN("Frame End");
            SDL_GL_SwapWindow(window);
        }
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
