#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>
#include <iostream>

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

    // 3. IME 核心配置（逐行确认，一个都不能少）
    // 强制显示IME UI（关键中的关键）
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
    // 开启文本输入（必须在窗口创建后）
    SDL_StartTextInput();
    // 设置IME候选框位置（必须在窗口可视区域内）
    SDL_Rect ime_rect = {0, 0, 0, 0}; // x=50,y=50 是窗口内坐标
    SDL_SetTextInputRect(&ime_rect);

    // 4. 事件循环（仅处理退出和文本输入）
    bool running = true;
    SDL_Event e;
    while (running) {
        // 确保事件循环不阻塞（SDL_PollEvent 必须循环处理）
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_TEXTINPUT:
                std::cout << "Input: " << e.text.text << std::endl; // 验证输入能被捕获
                break;
            case SDL_TEXTEDITING:
                std::cout << "Editing: " << e.edit.text << std::endl; // 验证编辑事件
                break;
            }
        }
        SDL_Delay(10); // 降低CPU占用，不影响事件处理
    }

    // 5. 清理
    SDL_StopTextInput();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}