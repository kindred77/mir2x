#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <iostream>

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

int main(int argc, char* argv[]) {
    // 1. 初始化仅视频模块（避免其他模块干扰）
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0) {
        std::cerr << "SDL Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // 2. 创建最简窗口（无任何额外标志）
    SDL_Window* window = SDL_CreateWindow(
        "SDL IME Test",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        600, 400,
        SDL_WINDOW_SHOWN  // 仅保留显示标志，不要加SDL_WINDOW_INPUT_FOCUS等
    );
    if (!window) {
        std::cerr << "Window create failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        std::cerr << "Renderer create failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // 3. IME 核心配置（逐行确认，一个都不能少）
    // 强制显示IME UI（关键中的关键）
    //SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
    // 开启文本输入（必须在窗口创建后）
    //SDL_StartTextInput();
    // 设置IME候选框位置（必须在窗口可视区域内）
    //SDL_Rect ime_rect = {0, 0, 0, 0}; // x=50,y=50 是窗口内坐标
    //SDL_SetTextInputRect(&ime_rect);

    IMGUI_CHECKVERSION();
    auto imgui_context = ImGui::CreateContext();

    //io.ConfigInputTextEnterKeepActive=true;
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    if (!ImGui_ImplSDL2_InitForSDLRenderer(window, renderer)
        || !ImGui_ImplSDLRenderer2_Init(renderer)) {
        std::cerr << "Can not init imgui: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    static bool no_titlebar = true;
    static bool no_scrollbar = true;
    static bool no_menu = true;
    static bool no_move = true;
    static bool no_resize = true;
    static bool no_collapse = true;
    static bool no_close = true;
    static bool no_nav = true;
    static bool no_background = true;
    static bool no_bring_to_front = true;
    static bool unsaved_document = true;
    static bool no_saved_settings = true;

    bool * p_open = NULL;

    int x = 50;
    int y = 50;
    int width = 100;
    int height = 30;
    ImGuiWindowFlags window_flags = 0;
    if (no_titlebar)        window_flags |= ImGuiWindowFlags_NoTitleBar;
    if (no_scrollbar)       window_flags |= ImGuiWindowFlags_NoScrollbar;
    if (!no_menu)           window_flags |= ImGuiWindowFlags_MenuBar;
    if (no_move)            window_flags |= ImGuiWindowFlags_NoMove;
    if (no_resize)          window_flags |= ImGuiWindowFlags_NoResize;
    if (no_collapse)        window_flags |= ImGuiWindowFlags_NoCollapse;
    if (no_nav)             window_flags |= ImGuiWindowFlags_NoNav;
    if (no_background)      window_flags |= ImGuiWindowFlags_NoBackground;
    if (no_bring_to_front)  window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    if (unsaved_document)   window_flags |= ImGuiWindowFlags_UnsavedDocument;
    if (no_saved_settings)   window_flags |= ImGuiWindowFlags_NoSavedSettings;
    if (no_close)           p_open = NULL;

    ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_FirstUseEver);
    // const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    // ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
    // ImGui::SetNextWindowSize(ImVec2(128, 30), ImGuiCond_FirstUseEver);

    ImGuiStyle* style = &ImGui::GetStyle();
    ImVec4* colors = style->Colors;
    colors[ImGuiCol_FrameBg] = ImVec4(0, 0, 0, 255);
    colors[ImGuiCol_Text] = ImVec4(255, 255, 255, 255);
    bool isPassword = false;
    char buf[64];

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigInputTextCursorBlink=true;
    ImFont* font = io.Fonts->AddFontFromFileTTF("/home/kindred/mywork/projects/cpp/mir2x/cmake-build-debug/install/client/res/font/05_NSIMSUN.TTF", 15, nullptr, io.Fonts->GetGlyphRangesChineseFull());
    IM_ASSERT(font != nullptr);
    io.Fonts->Build();

    // 4. 事件循环（仅处理退出和文本输入）
    bool running = true;
    SDL_Event e;
     while (running) {
        // 确保事件循环不阻塞（SDL_PollEvent 必须循环处理）
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            switch (e.type) {
            case SDL_QUIT:
                goto end_outer_loop;
            //case SDL_TEXTINPUT:
            //    std::cout << "Input: " << e.text.text << std::endl; // 验证输入能被捕获
           //     break;
            //case SDL_TEXTEDITING:
            //    std::cout << "Editing: " << e.edit.text << std::endl; // 验证编辑事件
            //    break;
                default:
                break;
            }
        }

         ImGui::SetCurrentContext(imgui_context);
         ImGui_ImplSDLRenderer2_NewFrame();
         ImGui_ImplSDL2_NewFrame();
         ImGui::NewFrame();

         bool ret = ImGui::Begin("##", p_open, window_flags);
         if (!ret) {
             std::cerr << "ImGui::Begin failed: " << SDL_GetError() << std::endl;
             SDL_Quit();
             return 1;
         }

         ImGui::PushFont(font);
         ImGui::SetCursorPos(ImVec2(x, y));
         ImGui::SetNextItemWidth(width);
         ImGuiInputTextCallbackData cb_user_data;

         if (isPassword) {
             ret = ImGui::InputTextWithHint("##", "this is hint", buf, sizeof(buf), ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue/* , InputTextCallback, &cb_user_data */);
         }
         else {
             ret = ImGui::InputTextWithHint("##", "this is hint", buf, sizeof(buf), ImGuiInputTextFlags_EnterReturnsTrue/* , InputTextCallback, &cb_user_data */);
         }
         if (ret) {
             SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "-----%s-----", buf);
            std::cout << "-------" << buf << std::endl;
         }
         ImGui::PopFont();
         ImGui::End();

         ImGui::Render();
         ImDrawData * drawData = ImGui::GetDrawData();
         ImGui_ImplSDLRenderer2_RenderDrawData(drawData, renderer);

         SDL_RenderPresent(renderer);
    }

    end_outer_loop:
    // 5. 清理
    SDL_StopTextInput();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}