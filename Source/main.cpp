// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "Windows.h"
#include "Math.h"
#include "Threading.h"

#include <stdio.h>
#include <string>
#include <vector>

#include <SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

#define DEVELOPMENT 1



struct NameStatus {
    std::string name;
    bool enabled = false;
};



[[nodiscard]] inline ImVec2 HadamardProduct(const ImVec2& a, const ImVec2& b)
{
    return { a.x * b.x, a.y * b.y };
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
        string->resize(data->BufSize);
        data->Buf = string->data();
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


void NameStatusButtonAdd(const std::string& buttonName, std::string& text, std::vector<NameStatus>& data, int codeLine)
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
    if (ImGui::Button(fullButtonName.c_str()) && text.size())
    {
        NameStatus ns = {};
        ns.name = text;
        data.push_back(ns);
        text.clear();
    }
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

void ExecutionSection(const std::string& sectionTitle, std::vector<NameStatus>& data, std::string& inputString, int selectedIndex)
{
    std::string sectionTitleEvents = sectionTitle + " Events:";
    ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
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
            data.push_back({ inputString });
            inputString.clear();
        }
        if (data.size() == 0)
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
        ImVec2 tableSize = ImVec2(0, ImGui::GetTextLineHeightWithSpacing() * (Min(maxTableHeight, (int)data.size()) + 1)); 
        const int tableColumnCount = 2;
        if (ImGui::BeginTable("table_advanced", tableColumnCount, tableFlags, tableSize, 0.0f))
        {
            DEFER{ ImGui::EndTable(); };

            const ImGuiTableColumnFlags columnFlags = ImGuiTableColumnFlags_WidthFixed;
            assert(data.size());
            float longestText = 0;
            for (const auto& item : data)
            {
                ImVec2 textSize = ImGui::CalcTextSize(item.name.c_str());
                longestText = Max(longestText, textSize.x);
            }
            ImGui::TableSetupScrollFreeze(1, 0);
            ImGui::TableSetupColumn("Status",   columnFlags | ImGuiTableColumnFlags_NoHide);
            ImGui::TableSetupColumn("String",   columnFlags, longestText);

            //ImGui::PushButtonRepeat(true);
            //DEFER{ ImGui::PopButtonRepeat(); };
            {
                for (int row_n = 0; row_n < data.size(); row_n++)
                {
                    auto item = data[row_n];

                    ImGui::PushID(item.name.c_str());
                    DEFER{ ImGui::PopID(); };

                    ImGui::TableNextRow(ImGuiTableRowFlags_None, 0);
                    if (ImGui::TableSetColumnIndex(0))
                    {
                        std::string buttonLabel = "Disabled";
                        float color = 0.0f;
                        if (data[row_n].enabled)
                        {
                            color = 2.0f / 7.0f;
                            buttonLabel = "Enabled";
                        }
                        
                        ImGui::PushStyleColor(ImGuiCol_Button,          (ImVec4)ImColor::HSV(color, 0.6f, 0.6f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,   (ImVec4)ImColor::HSV(color, 0.7f, 0.7f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive,    (ImVec4)ImColor::HSV(color, 0.8f, 0.8f));
                        if (ImGui::SmallButton(buttonLabel.c_str()))
                        {
                            data[row_n].enabled = !data[row_n].enabled;
                        }
                        ImGui::PopStyleColor(3);
                    }


                    ImGui::TableSetColumnIndex(1);
                    ImGuiSelectableFlags selectable_flags = ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
                    ImGui::Selectable(item.name.c_str(), false, selectable_flags, ImVec2(0, 0));
                    if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
                    {
                        int nextIndex = row_n + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
                        if (nextIndex >= 0 && nextIndex < data.size())
                        {
                            data[row_n] = data[nextIndex];
                            data[nextIndex] = item;
                            ImGui::ResetMouseDragDelta();
                        }
                    }
                }
            }
        }
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
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    Threading& threading = Threading::GetInstance();
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);
    
    std::string mainPath;
    std::string projectPath;

    int platformSelection = 0;
    std::vector<NameStatus> platformOptions;
    platformOptions.push_back({ "Win64" });
    platformOptions.push_back({ "XboxOne" });
    platformOptions.push_back({ "XboxOneGDK" });
    platformOptions.push_back({ "XSX" });
    platformOptions.push_back({ "PS4" });
    platformOptions.push_back({ "PS5" });

    std::vector<NameStatus> versionOptions;
    versionOptions.push_back({ "Shipping" });
    versionOptions.push_back({ "Test" });
    versionOptions.push_back({ "Development" });
    versionOptions.push_back({ "Debug" });

    std::vector<NameStatus> switchOptions;
    switchOptions.push_back({ "AdditionalCookerOptions=\"-ddc=noshared\"" });
    switchOptions.push_back({ "AdditionalCookerOptions=\"-ddc=ddcreadonly\"" });
    switchOptions.push_back({ "distribution" });
    switchOptions.push_back({ "MapIniSectionsToCook=DevCookMaps" });
    switchOptions.push_back({ "cook" });
    switchOptions.push_back({ "NoP4" });
    switchOptions.push_back({ "manifests" });
    switchOptions.push_back({ "pak" });
    switchOptions.push_back({ "build" });
    switchOptions.push_back({ "stage" });
    switchOptions.push_back({ "compress" });
    switchOptions.push_back({ "dedicatedserver" });
    switchOptions.push_back({ "package" });
    switchOptions.push_back({ "skipcook" });
    switchOptions.push_back({ "skipbuild" });

    std::vector<NameStatus> preBuildEvents;
    std::vector<NameStatus> postBuildEvents;


    std::string finalCommandLine;
    bool buildRunning = false;

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
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
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
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        //ImGui::PushFont(mainFont);
        if (!threading.GetJobsInFlight())
            buildRunning = false;


        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos, ImGuiCond_Always, {});
        ImGui::SetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(1.0f); // Transparent background
        ImGuiWindowFlags windowFlags = 
            ImGuiWindowFlags_NoDecoration | 
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing | 
            ImGuiWindowFlags_NoNav | 
            ImGuiWindowFlags_NoMove;

        if (ImGui::Begin("Main", nullptr, windowFlags))
        {
            //DEFER{ ImGui::End(); };
            ImGuiWindowFlags sectionFlags = 
#if 1
                ImGuiWindowFlags_NoResize | 
#else
                ImGuiWindowFlags_AlwaysAutoResize |
#endif
                ImGuiWindowFlags_NoCollapse | 
                ImGuiWindowFlags_NoFocusOnAppearing | 
                ImGuiWindowFlags_NoNav | 
                ImGuiWindowFlags_NoMove;

            ImVec2 locationScale = { 0.7f, 0.1f };
            ImVec2 locationSize = HadamardProduct(viewport->WorkSize, locationScale);
            ImGui::SetNextWindowPos(ImVec2((((1 - locationScale.x) / 2) * viewport->WorkSize.x), locationSize.y), 0, ImVec2(0.0f, 1.0f));
            if (ImGui::BeginChild("Location", locationSize, true, sectionFlags | ImGuiWindowFlags_NoScrollbar))
            {
                //DEFER{ ImGui::EndChild(); };
#ifdef DEBUG
                ImGui::Checkbox("Show Demo Window", &show_demo_window);
                ImGui::SameLine();
#endif
                TextCentered("File Paths");

                ImGui::Text("Main Dir");
                ImGui::SameLine();
                std::string demoMainDir = "C:/UnrealEngine/Project";
                InputTextDynamicSize("##" + demoMainDir, mainPath);
                ImGui::SameLine();
                HelpMarker(demoMainDir);

                ImGui::Text("Path to Project File");
                ImGui::SameLine();
                std::string demoProjectPath = "C:/UnrealEngine/Project/Title/title.uproject";
                InputTextDynamicSize("##" + demoProjectPath, projectPath);
                ImGui::SameLine();
                HelpMarker(demoProjectPath);
            }
            ImGui::EndChild();
            ImVec2 platformScale = { 0.7f, 0.2f };
            ImVec2 platformSize = HadamardProduct(viewport->WorkSize, platformScale);
            ImGui::SetNextWindowPos(ImVec2((((1 - platformScale.x) / 2) * viewport->WorkSize.x), locationSize.y), 0, ImVec2(0.0f, 0.0f));
            if (ImGui::BeginChild("Platform", platformSize, true, sectionFlags))
            {
                {
                    TextCentered("Platform Selection");
                    ImGui::NewLine();
                    float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
                    float last_button_x2 = 0;
                    ImGuiStyle& style = ImGui::GetStyle();
                    for (int i = 0; i < platformOptions.size(); i++)
                    {
                        float button_szx = ImGui::CalcTextSize(platformOptions[i].name.c_str()).x + 2 * style.FramePadding.x;
                        float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_szx;
                        if (next_button_x2 < window_visible_x2)
                            ImGui::SameLine(0, 1);
                        ImGui::RadioButton(platformOptions[i].name.c_str(), &platformSelection, i);
                        last_button_x2 = ImGui::GetItemRectMax().x;
                    }
                    static std::string name;
                    NameStatusButtonAdd("Platform", name, platformOptions, __LINE__);
                }

                {
                    ImGui::NewLine();
                    TextCentered("Version Selection");
                    ImGui::NewLine();
                    float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
                    float last_button_x2 = 0;
                    ImGuiStyle& style = ImGui::GetStyle();
                    for (int i = 0; i < versionOptions.size(); i++)
                    {
                        float button_szx = ImGui::CalcTextSize(versionOptions[i].name.c_str()).x + 2 * style.FramePadding.x;
                        float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_szx;
                        if (next_button_x2 < window_visible_x2)
                            ImGui::SameLine(0, 1);
                        ImGui::Checkbox(versionOptions[i].name.c_str(), &versionOptions[i].enabled);
                        last_button_x2 = ImGui::GetItemRectMax().x;
                    }
                    static std::string name;
                    NameStatusButtonAdd("Version", name, versionOptions, __LINE__);
                }

                //ImGui::Text("Platform Selection");
            }
            ImGui::EndChild();

            ImVec2 switchesScale = { 0.7f, 0.2f };
            ImVec2 switchesSize = HadamardProduct(viewport->WorkSize, switchesScale);
            ImGui::SetNextWindowPos(ImVec2((((1 - switchesScale.x) / 2) * viewport->WorkSize.x), locationSize.y + platformSize.y), 0, ImVec2(0.0f, 0.0f));
            if (ImGui::BeginChild("Switches", switchesSize, true, sectionFlags))
            {
                TextCentered("Switch Selection");
                ImGui::NewLine();
                float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
                float last_button_x2 = 0;
                ImGuiStyle& style = ImGui::GetStyle();
                for (int i = 0; i < switchOptions.size(); i++)
                {
                    float button_szx = ImGui::CalcTextSize(switchOptions[i].name.c_str()).x + 2 * style.FramePadding.x;
                    float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_szx;
                    if (next_button_x2 < window_visible_x2)
                        ImGui::SameLine(0, 1);
                    ImGui::Checkbox(switchOptions[i].name.c_str(), &switchOptions[i].enabled);
                    last_button_x2 = ImGui::GetItemRectMax().x;
                }
                static std::string name;
                NameStatusButtonAdd("Switch", name, switchOptions, __LINE__);
            }
            ImGui::EndChild();

            ImVec2 executionScale = { 0.7f, 0.3f };
            ImVec2 executionSize = HadamardProduct(viewport->WorkSize, executionScale);
            ImGui::SetNextWindowPos(ImVec2((((1 - executionScale.x) / 2) * viewport->WorkSize.x), locationSize.y + platformSize.y + switchesSize.y), 0, ImVec2(0, 0.0f));
            if (ImGui::BeginChild("Execution", executionSize, true, sectionFlags))
            {
                TextCentered("Execution Settings");

                static std::string preBuildInput;
                static int preBuildSelectionIndex = -1;
                ExecutionSection("Pre-Build", preBuildEvents, preBuildInput, preBuildSelectionIndex);

                static std::string postBuildInput;
                static int postBuildSelectionIndex = -1;
                ExecutionSection("Post-Build", postBuildEvents, postBuildInput, postBuildSelectionIndex);
            }
            ImGui::EndChild();

            bool commandLineInvalid = projectPath.size() < 3;
            ImVec2 commandScale = { 0.7f, 0.2f };
            ImVec2 commandSize = HadamardProduct(viewport->WorkSize, commandScale);
            ImGui::SetNextWindowPos(ImVec2((((1 - commandScale.x) / 2) * viewport->WorkSize.x), locationSize.y + platformSize.y + switchesSize.y + executionSize.y), 0, ImVec2(0.0f, 0.0f));
            if (ImGui::BeginChild("Command Line", commandSize, true, sectionFlags))
            {

                //static std::string finalCommandLine = "This is just a test commandline output lmao QWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW";
                finalCommandLine.clear();
                if (commandLineInvalid)
                {
                    finalCommandLine = "Please add a path to Project Path";
                }
                else
                {
                    finalCommandLine += mainPath.c_str();
                    finalCommandLine += "/Engine/Build/BatchFiles/RunUAT.bat BuildCookRun ";
                    finalCommandLine += "-project=";
                    finalCommandLine += projectPath.c_str();
                    finalCommandLine += " -targetplatform=";
                    finalCommandLine += platformOptions[platformSelection].name;
                    finalCommandLine += " -clientconfig=";
                    bool alreadyOneEnabled = false;
                    for (const auto& option : versionOptions)
                    {
                        if (option.enabled)
                        {
                            if (alreadyOneEnabled)
                                finalCommandLine += "+";
                            finalCommandLine += option.name;
                            alreadyOneEnabled = true;
                        }
                    }
                    finalCommandLine += " -servertargetplatform=win64";
                    finalCommandLine += " -serverconfig=Development";

                    for (const auto& option : switchOptions)
                    {
                        if (option.enabled)
                        {
                            finalCommandLine += " -";
                            finalCommandLine += option.name;
                        }
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
                    s32 enabledPreBuildEventsCount = 0;
                    for (s32 i = 0; i < preBuildEvents.size(); i++)
                    {
                        if (preBuildEvents[i].enabled)
                        {
                            std::string path;
                            std::string args;
                            SeperatePathAndArguments(preBuildEvents[i].name, path, args);
                            RemoveStartAndEndSpaces(path);
                            if (args.size())
                                RemoveStartAndEndSpaces(args);
                            enabledPreBuildEventsCount++;
                            StartProcessJob* job = new StartProcessJob();
                            job->applicationPath = path;
                            job->arguments = args;
                            threading.SubmitJob(job);
                        }
                    }
                    s32 enabledPostBuildEventsCount = 0;
                    for (s32 i = 0; i < postBuildEvents.size(); i++)
                    {
                        if (postBuildEvents[i].enabled)
                            enabledPostBuildEventsCount++;
                    }
                }
                if (buildRunning || commandLineInvalid)
                    ImGui::EndDisabled();
                ImGui::SameLine();
                ImGui::BeginDisabled();
                if (ImGui::Button("Save Config"))
                {

                }
                ImGui::EndDisabled();
                ImGui::SameLine();
                ImGui::BeginDisabled();
                if (ImGui::Button("Export"))
                {

                }
                ImGui::EndDisabled();
            }
            ImGui::EndChild();
            ImGui::End();
        }

        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // Rendering
        //ImGui::PopFont();
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
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
