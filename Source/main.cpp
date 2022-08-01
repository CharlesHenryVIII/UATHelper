// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include <stdio.h>
#include <string>
#include <vector>

#include <SDL.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif




struct NameStatus {
    std::string name;
    bool enabled = false;
};




void TextCentered(std::string text) {
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


[[nodiscard]] inline ImVec2 HadamardProduct(const ImVec2& a, const ImVec2& b)
{
    return { a.x * b.x, a.y * b.y };
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
//NOTE: Ideally there would be a function overload that takes a const char* for the title input but the function cannot be determined
//since passsing a string could convert to a const char* or be built into a std::string
bool InputTextDynamicSize(const std::string& title, std::string& s, ImGuiInputTextFlags flags = ImGuiInputTextFlags_None)
{
    return ImGui::InputText(title.c_str(), s.data(), s.capacity(), flags | ImGuiInputTextFlags_CallbackResize, DynamicTextCallback, &s);
}
bool InputTextMultilineDynamicSize(const std::string& title, std::string& s, ImGuiInputTextFlags flags = ImGuiInputTextFlags_None)
{
    return ImGui::InputTextMultiline(title.c_str(), const_cast<char*>(title.data()), s.capacity(), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 2), flags | ImGuiInputTextFlags_CallbackResize, DynamicTextCallback, &s);
}


#if 0
void ExecutionSection(const std::string& sectionTitle, std::vector<NameStatus>& data, std::string& inputString, int selectedIndex)
#else
void ExecutionSection(const std::string& sectionTitle, std::vector<NameStatus>& data, std::string& inputString)
#endif
{
    std::string sectionTitleEvents = sectionTitle + " Events:";
    if (ImGui::TreeNode(sectionTitleEvents.c_str()))
    {
        bool inputSuccess = false;
        std::string textInputTitle = "##" + sectionTitle + " Event";
        inputSuccess |= InputTextDynamicSize(textInputTitle.c_str(), inputString, ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();
        inputSuccess |= ImGui::Button("Add");
        if (inputSuccess)
        {
            data.push_back({ inputString });
            inputString.clear();
        }

#if 1

        ImGuiTableFlags tableFlags = 0 |
            ImGuiTableFlags_RowBg |
            ImGuiTableFlags_BordersV |
            ImGuiTableFlags_BordersOuterV |
            ImGuiTableFlags_BordersInnerV |
            ImGuiTableFlags_BordersH |
            ImGuiTableFlags_BordersOuterH |
            ImGuiTableFlags_BordersInnerH |
            //ImGuiTableFlags_SizingStretchProp |
            //ImGuiTableFlags_SizingFixedFit | 
            //ImGuiTableFlags_SizingStretchSame | 
            ImGuiTableFlags_ScrollX | 
            //ImGuiTableFlags_NoBordersInBody |
            //ImGuiTableFlags_Resizable |
            ImGuiTableFlags_NoSavedSettings;
            //ImGuiTableFlags_NoBordersInBodyUntilResize;

        const int tableColumnCount = 3;
        if (ImGui::BeginTable("table_advanced", tableColumnCount, tableFlags, ImVec2(0, 0), 0.0f))
        {
            // Declare columns
            // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
            // This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!

            ImGuiTableColumnFlags columnFlags = 0;
                //ImGuiTableColumnFlags_DefaultSort |
                //ImGuiTableColumnFlags_WidthFixed |
                //ImGuiTableColumnFlags_NoHide;
            ImGui::TableSetupColumn("String",   columnFlags | ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Status",   columnFlags | ImGuiTableColumnFlags_WidthFixed /*, 70.0f*/);    //TODO: Fix magic numbers
            ImGui::TableSetupColumn("Buttons",  columnFlags | ImGuiTableColumnFlags_WidthFixed /*, 95.0f*/);   //TODO: Fix magic numbers
            int freeze_cols, freeze_rows;
            freeze_cols = freeze_rows = 1;
            ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows); //??

            //ImGui::TableHeadersRow();

            // Show data
            // FIXME-TABLE FIXME-NAV: How we can get decent up/down even though we have the buttons here?
            ImGui::PushButtonRepeat(true);
            {
                //ImGui::TableHeadersRow();
                for (int row_n = 0; row_n < data.size(); row_n++)
                {
                    auto item = data[row_n];

                    ImGui::PushID(item.name.c_str());
                    ImGui::TableNextRow(ImGuiTableRowFlags_None, 0);

                    ImGui::TableSetColumnIndex(0);
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

                    if (ImGui::TableSetColumnIndex(1))
                    {
                        std::string buttonLabel = "disabled";
                        float color = 0.0f;
                        if (data[row_n].enabled)
                        {
                            color = 2.0f / 7.0f;
                            buttonLabel = "enabled";
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

                    if (ImGui::TableSetColumnIndex(2))
                    {
                        ImGui::SmallButton("Edit"); ImGui::SameLine();

                        ImGui::PushStyleColor(ImGuiCol_Button,          (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,   (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive,    (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
                        ImGui::SmallButton("Delete");
                        ImGui::PopStyleColor(3);
                    }


                    ImGui::PopID();
                }
            }
            ImGui::PopButtonRepeat();
            ImGui::EndTable();
        }
        ImGui::TreePop();
#else
        for (int i = 0; i < data.size(); i++)
        {
            auto currentCopy = data[i];
            ImGui::Selectable(currentCopy.name.c_str());
            ImGui::SameLine();
            ImGui::Button("Edit");

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
            ImGui::Button("Delete");
            ImGui::PopStyleColor(3);

            if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
            {
                int nextIndex = i + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
                if (nextIndex >= 0 && nextIndex < data.size())
                {
                    data[i] = data[nextIndex];
                    data[nextIndex] = currentCopy;
                    ImGui::ResetMouseDragDelta();
                }
            }
        }
        ImGui::TreePop();
#endif
    }
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

    int platformSelection = 0;
    std::vector<std::string> platformOptions;
    platformOptions.push_back("Win64");
    platformOptions.push_back("XboxOne");
    platformOptions.push_back("XboxOneGDK");
    platformOptions.push_back("XSX");
    platformOptions.push_back("PS4");
    platformOptions.push_back("PS5");

    std::vector<NameStatus> versionOptions;
    versionOptions.push_back({ "Test" });
    versionOptions.push_back({ "Debug" });
    versionOptions.push_back({ "Shipping" });
    versionOptions.push_back({ "Development" });

    std::vector<NameStatus> switchOptions;
    switchOptions.push_back({"DDC=NoShared"});
    switchOptions.push_back({"cook"});
    switchOptions.push_back({"package"});
    switchOptions.push_back({"Nativize"});

    std::vector<NameStatus> preBuildEvents;
    preBuildEvents.push_back({"C:/Users/tonyk/Documents/Apps/cloc-1.90.exe C:/Projects/UATHelper/Source"});
    preBuildEvents.push_back({"AAAAAAAAAAAAAAAAAAAAAAaaaaaaaaaaaaaaaaaaaaaaaAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                              "AAAAAAAAAAAAAAAAAAAAAAAAAaaaaaaaaaaAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                              "aaaaaaAAAAAAAAAAAAAAAAAAAaaaaaaaaaaAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"});
    std::vector<NameStatus> postBuildEvents;

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
    bool show_demo_window = true;
    bool show_another_window = false;
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

            ImVec2 platformScale = { 0.5f, 0.2f };
            ImVec2 platformSize = HadamardProduct(viewport->WorkSize, platformScale);
            ImGui::SetNextWindowPos(platformSize, 0, ImVec2(0.5f, 1.0f));
            if (ImGui::BeginChild("Platform", ImVec2(viewport->WorkSize.x / 2, platformSize.y), true, sectionFlags))
            {
                {
                    TextCentered("Platform Selection");
                    ImGui::NewLine();
                    float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
                    float last_button_x2 = 0;
                    ImGuiStyle& style = ImGui::GetStyle();
                    for (int i = 0; i < platformOptions.size(); i++)
                    {
                        float button_szx = ImGui::CalcTextSize(platformOptions[i].c_str()).x + 2 * style.FramePadding.x;
                        float next_button_x2 = last_button_x2 + style.ItemSpacing.x + button_szx;
                        if (next_button_x2 < window_visible_x2)
                            ImGui::SameLine(0, 1);
                        ImGui::RadioButton(platformOptions[i].c_str(), &platformSelection, i);
                        last_button_x2 = ImGui::GetItemRectMax().x;
                    }
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
                }

                //ImGui::Text("Platform Selection");
                ImGui::EndChild();
            }

            ImVec2 switchesScale = { 0.5f, 0.2f };
            ImVec2 switchesSize = HadamardProduct(viewport->WorkSize, switchesScale);
            ImGui::SetNextWindowPos(ImVec2({ viewport->WorkSize.x / 2, platformSize.y + switchesSize.y / 2 }), 0, ImVec2(0.5f, 0.5f));
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
                ImGui::EndChild();
            }

            ImVec2 executionScale = { 0.7f, 0.15f };
            ImVec2 executionSize = HadamardProduct(viewport->WorkSize, executionScale);
            ImGui::SetNextWindowPos(ImVec2({ viewport->WorkSize.x / 2, platformSize.y + switchesSize.y }), 0, ImVec2(0.5f, 0.0f));
            if (ImGui::BeginChild("Execution", executionSize, true, sectionFlags))
            {
                TextCentered("Execution Settings");

#if 1
                static std::string preBuildInput;
                ExecutionSection("Pre-Build", preBuildEvents, preBuildInput);

                static std::string postBuildInput;
                ExecutionSection("Post-Build", postBuildEvents, postBuildInput);
#else
                std::string sectionTitle = "Pre-Build";
                if (ImGui::TreeNode("%s Events:", sectionTitle.c_str()))
                {
                    //ImGui::Text("%s Events:", sectionTitle.c_str());
                    //ImGui::SameLine();

                    static std::string preBuildInput;
                    bool preBuildSuccess = false;
                    std::string textInputTitle = "##" + sectionTitle + " Event";
                    preBuildSuccess |= InputTextDynamicSize(textInputTitle.c_str(), preBuildInput, ImGuiInputTextFlags_EnterReturnsTrue);
                    ImGui::SameLine();
                    preBuildSuccess |= ImGui::Button("Add");
                    if (preBuildSuccess)
                    {
                        preBuildInput;
                        preBuildEvents.push_back({ preBuildInput });
                        preBuildInput.clear();
                    }

                    for (int i = 0; i < preBuildEvents.size(); i++)
                    {
                        auto currentCopy = preBuildEvents[i];
                        ImGui::Selectable(currentCopy.name.c_str());
                        if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
                        {
                            int nextIndex = i + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
                            if (nextIndex >= 0 && nextIndex < preBuildEvents.size())
                            {
                                preBuildEvents[i] = preBuildEvents[nextIndex];
                                preBuildEvents[nextIndex] = currentCopy;
                                ImGui::ResetMouseDragDelta();
                            }
                        }
                    }
                    ImGui::TreePop();
                }
#endif
                ImGui::EndChild();
            }
            ImGui::End();
        }

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        //{
        //    static float f = 0.0f;
        //    static int counter = 0;

        //    ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        //    ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        //    ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        //    ImGui::Checkbox("Another Window", &show_another_window);

        //    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        //    ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        //    if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        //        counter++;
        //    ImGui::SameLine();
        //    ImGui::Text("counter = %d", counter);

        //    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        //    ImGui::End();
        //}

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

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
