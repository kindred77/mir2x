#include <cmath>
#include <utf8.h>
#include "mathf.hpp"
#include "colorf.hpp"
#include "imeboard.hpp"
#include "inputline.hpp"
#include "sdldevice.hpp"
#include "labelboard.hpp"
#include "clientargparser.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

extern IMEBoard *g_imeBoard;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

InputLine::InputLine(InputLine::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = std::move(args.w),
          .h = std::move(args.h),

          .parent = std::move(args.parent),
      }}

    , m_imeEnabled(std::move(args.enableIME))
    , m_tpset
      {
          0,
          LALIGN_LEFT,
          false,

          args.font.id,
          args.font.size,
          args.font.style,

          std::move(args.font.color),
          std::move(args.font.bgColor),
      }

    , m_cursorArgs(std::move(args.cursor))

    , m_onTab   (std::move(args.onTab))
    , m_onCR    (std::move(args.onCR))
    , m_onChange(std::move(args.onChange))
    , m_validate(std::move(args.validate))
{
    if (Widget::evalBool(m_imeEnabled, this)){
        IMGUI_CHECKVERSION();
        m_imgui_context = ImGui::CreateContext();

        //io.ConfigInputTextEnterKeepActive=true;
        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        auto sdl_renderer = g_sdlDevice->getRenderer();
        auto sdl_window = g_sdlDevice->getWindow();
        if (!ImGui_ImplSDL2_InitForSDLRenderer(sdl_window, sdl_renderer)
            || !ImGui_ImplSDLRenderer2_Init(sdl_renderer)) {
            std::cerr << "Can not init imgui: " << SDL_GetError() << std::endl;
            return;
        }

        static bool no_titlebar = true;
        static bool no_scrollbar = true;
        static bool no_menu = true;
        static bool no_move = true;
        static bool no_resize = true;
        static bool no_collapse = true;
        static bool no_nav = true;
        static bool no_background = false;
        static bool no_bring_to_front = true;
        static bool unsaved_document = true;
        static bool no_saved_settings = true;

        //bool * p_open = NULL;

        if (no_titlebar)        m_window_flags |= ImGuiWindowFlags_NoTitleBar;
        if (no_scrollbar)       m_window_flags |= ImGuiWindowFlags_NoScrollbar;
        if (!no_menu)           m_window_flags |= ImGuiWindowFlags_MenuBar;
        if (no_move)            m_window_flags |= ImGuiWindowFlags_NoMove;
        if (no_resize)          m_window_flags |= ImGuiWindowFlags_NoResize;
        if (no_collapse)        m_window_flags |= ImGuiWindowFlags_NoCollapse;
        if (no_nav)             m_window_flags |= ImGuiWindowFlags_NoNav;
        if (no_background)      m_window_flags |= ImGuiWindowFlags_NoBackground;
        if (no_bring_to_front)  m_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
        if (unsaved_document)   m_window_flags |= ImGuiWindowFlags_UnsavedDocument;
        if (no_saved_settings)   m_window_flags |= ImGuiWindowFlags_NoSavedSettings;
        //if (no_close)           p_open = NULL;

        //ImGui::SetNextWindowPos(ImVec2(159, 400), ImGuiCond_FirstUseEver);
        //ImGui::SetNextWindowSize(ImVec2(146, 18), ImGuiCond_FirstUseEver);
        // const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        // ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
        // ImGui::SetNextWindowSize(ImVec2(128, 30), ImGuiCond_FirstUseEver);

        ImGuiStyle* style = &ImGui::GetStyle();
        ImVec4* colors = style->Colors;
        colors[ImGuiCol_FrameBg] = ImVec4(100, 0, 0, 255);
        colors[ImGuiCol_Text] = ImVec4(255, 255, 255, 255);
        //std::memset(m_input_buf, 0, sizeof(m_input_buf));
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigInputTextCursorBlink=true;
        m_font = io.Fonts->AddFontFromFileTTF("/home/kindred/mywork/projects/cpp/mir2x/cmake-build-debug/install/client/res/font/05_NSIMSUN.TTF", 15, nullptr, io.Fonts->GetGlyphRangesChineseFull());
        IM_ASSERT(m_font != nullptr);
        io.Fonts->Build();
    }

}

bool InputLine::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }
    if (Widget::evalBool(m_imeEnabled, this)){
        ImGui_ImplSDL2_ProcessEvent(&event);
    }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                // another widget can consume the event
                // and pass the focus to this widget, don't drop focus for keyboard events

                if(!valid){
                    return false;
                }

                if(!focus()){
                    return false;
                }

                switch(event.key.keysym.sym){
                    case SDLK_TAB:
                        {
                            if(m_onTab){
                                m_onTab();
                            }
                            return true;
                        }
                    case SDLK_RETURN:
                        {
                            if(m_onCR){
                                m_onCR();
                            }
                            return true;
                        }
                    case SDLK_LEFT:
                        {
                            m_cursor = std::max<int>(0, m_cursor - 1);
                            m_cursorBlink = 0.0;
                            return true;
                        }
                    case SDLK_RIGHT:
                        {
                            if(m_tpset.empty()){
                                m_cursor = 0;
                            }
                            else{
                                m_cursor = std::min<int>(m_tpset.lineTokenCount(0), m_cursor + 1);
                            }
                            m_cursorBlink = 0.0;
                            return true;
                        }
                    case SDLK_BACKSPACE:
                        {
                            if(m_cursor > 0){
                                m_tpset.deleteToken(m_cursor - 1, 0, 1);
                                m_cursor--;

                                if(m_onChange){
                                    m_onChange(m_tpset.getRawString());
                                }
                            }
                            m_cursorBlink = 0.0;
                            return true;
                        }
                    case SDLK_ESCAPE:
                        {
                            setFocus(false);
                            return true;
                        }
                    default:
                        {
                            if(!Widget::evalBool(m_imeEnabled, this)){
                                const char keyChar = SDLDeviceHelper::getKeyChar(event, true);
                                if(keyChar != '\0'){
                                    m_tpset.insertUTF8String(m_cursor++, 0, str_printf("%c", keyChar).c_str());
                                    if(m_onChange){
                                        m_onChange(m_tpset.getRawString());
                                    }
                                }

                                m_cursorBlink = 0.0;
                                return true;
                            }
                            else{
                                return false;
                            }


                            // if(!g_clientArgParser->disableIME && Widget::evalBool(m_imeEnabled, this) && g_imeBoard->active() && (keyChar >= 'a' && keyChar <= 'z')){
                            //     g_imeBoard->gainFocus("", str_printf("%c", keyChar), this, [this](std::string s)
                            //     {
                            //         m_tpset.insertUTF8String(m_cursor, 0, s.c_str());
                            //         m_cursor += utf8::distance(s.begin(), s.end());
                            //         if(m_onChange){
                            //             m_onChange(m_tpset.getRawString());
                            //         }
                            //     });
                            // }
                            // else if(keyChar != '\0'){
                            //     m_tpset.insertUTF8String(m_cursor++, 0, str_printf("%c", keyChar).c_str());
                            //     if(m_onChange){
                            //         m_onChange(m_tpset.getRawString());
                            //     }
                            // }
                            //
                            // m_cursorBlink = 0.0;
                            // return true;
                        }
                }
            }
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            {
                if(!valid){
                    return consumeFocus(false);
                }

                if(!m.in(event.button.x, event.button.y)){
                    return consumeFocus(false);
                }

                if(event.type == SDL_MOUSEBUTTONDOWN){
                    const int eventX = event.button.x - m.x;
                    const int eventY = event.button.y - m.y;

                    const auto [cursorX, cursorY] = m_tpset.locCursor(eventX, eventY);
                    if(cursorY != 0){
                        throw fflerror("cursor locates at wrong line");
                    }

                    m_cursor = cursorX;
                    m_cursorBlink = 0.0;
                }

                return consumeFocus(true);
            }
        // case SDL_TEXTINPUT:
        //     {
        //         if (Widget::evalBool(m_imeEnabled, this)) {
        //             auto &&in_str = std::string(event.text.text);
        //             m_tpset.insertUTF8String(m_cursor, 0, str_printf("%s", in_str.c_str()).c_str());
        //             if(m_onChange){
        //                 m_onChange(m_tpset.getRawString());
        //             }
        //             m_cursor += utf8::distance(in_str.begin(), in_str.end());
        //             m_cursorBlink = 0.0;
        //             return true;
        //         }
        //         return false;
        //     }
        default:
            {
                return false;
            }
    }
}

void InputLine::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    int dstCropX = m.x;
    int dstCropY = m.y;
    int srcCropX = m.ro->x;
    int srcCropY = m.ro->y;
    int srcCropW = m.ro->w;
    int srcCropH = m.ro->h;

    const int tpsetX = 0;
    const int tpsetY = 0 + (h() - (m_tpset.empty() ? m_tpset.getDefaultFontHeight() : m_tpset.ph())) / 2;

    const auto needDraw = mathf::cropROI(
            &srcCropX, &srcCropY,
            &srcCropW, &srcCropH,
            &dstCropX, &dstCropY,

            w(),
            h(),

            tpsetX, tpsetY, m_tpset.pw(), m_tpset.ph());

    if(needDraw){
        m_tpset.draw({.x=dstCropX, .y=dstCropY, .ro{srcCropX - tpsetX, srcCropY - tpsetY, srcCropW, srcCropH}});
    }

    if(Widget::evalBool(m_imeEnabled, this)){
        ImGui::SetCurrentContext(m_imgui_context);
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::SetCursorPos(ImVec2(159, 400));
        ImGui::Dummy(ImVec2(159, 400));
        ImGui::SetNextItemWidth(146);


        bool ret = ImGui::Begin("##", NULL, m_window_flags);
        if (!ret) {
            std::cerr << "ImGui::Begin failed: " << SDL_GetError() << std::endl;
            return;
        }

        ImGui::PushFont(m_font);
        //ImGui::SetCursorPos(ImVec2(dstCropX, dstCropY));
        //ImGui::SetNextItemWidth(srcCropW);
        ImGui::SetCursorPos(ImVec2(0, 0));
        ImGui::SetNextItemWidth(146);
        ImGuiInputTextCallbackData cb_user_data;

        //if (isPassword) {
        //ret = ImGui::InputTextWithHint("##", "this is hint", (char *)m_input_buf, sizeof(m_input_buf), ImGuiInputTextFlags_Password | ImGuiInputTextFlags_EnterReturnsTrue/* , InputTextCallback, &cb_user_data */);
        //}
        //else {
        ret = ImGui::InputTextWithHint("##", "this is hint", (char *)m_input_buf, sizeof(m_input_buf), ImGuiInputTextFlags_EnterReturnsTrue/* , InputTextCallback, &cb_user_data */);
        //}
        if (ret) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "-----%s-----", m_input_buf);
            std::cout << "-------" << m_input_buf << std::endl;
        }
        ImGui::PopFont();
        ImGui::End();

        ImGui::Render();
        ImDrawData * drawData = ImGui::GetDrawData();
        ImGui_ImplSDLRenderer2_RenderDrawData(drawData, g_sdlDevice->getRenderer());

        g_sdlDevice->drawRectangle(colorf::BLUE + colorf::A_SHF(255), m.x, m.y, w(), h());
        return;
    }

    if(std::fmod(m_cursorBlink, 1000.0) > 500.0){
        return;
    }

    if(!focus()){
        return;
    }

    int cursorY = m.y + tpsetY;
    int cursorX = m.x + tpsetX + [this]()
    {
        if(m_tpset.empty() || m_cursor == 0){
            return 0;
        }

        if(m_cursor == m_tpset.lineTokenCount(0)){
            return m_tpset.pw();
        }

        const auto pToken = m_tpset.getToken(m_cursor - 1, 0);
        return pToken->box.state.w1 + pToken->box.state.x + pToken->box.info.w;
    }();

    int cursorW = Widget::evalSizeOpt(m_cursorArgs.w, this, []{ return 2; });
    int cursorH = std::max<int>(m_tpset.ph(), h());

    if(mathf::rectangleOverlapRegion(m.x, m.y, m.ro->w, m.ro->h, cursorX, cursorY, cursorW, cursorH)){
        g_sdlDevice->fillRectangle(Widget::evalU32(m_cursorArgs.color, this), cursorX, cursorY, cursorW, cursorH);
    }

    //if(g_clientArgParser->debugDrawInputLine){
        g_sdlDevice->drawRectangle(colorf::BLUE + colorf::A_SHF(255), m.x, m.y, w(), h());
    //}
}

void InputLine::deleteChar()
{
    m_tpset.deleteToken(m_cursor - 1, 0, 1);
    m_cursor--;
}

void InputLine::insertChar(char ch)
{
    const char rawString[]
    {
        ch, '\0',
    };

    m_tpset.insertUTF8String(m_cursor, 0, rawString);
    m_cursor++;
}

void InputLine::insertUTF8String(const char *utf8Str)
{
    if(str_haschar(utf8Str)){
        m_cursor += m_tpset.insertUTF8String(m_cursor, 0, utf8Str);
    }
}

void InputLine::clear()
{
    m_cursor = 0;
    m_cursorBlink = 0.0;

    if(!m_tpset.empty()){
        m_tpset.clear();

        if(m_onChange){
            m_onChange({});
        }
    }
}

void InputLine::setInput(const char *utf8Str)
{
    m_cursor = 0;
    m_cursorBlink = 0.0;

    m_tpset.clear();
    if(str_haschar(utf8Str)){
        m_cursor = m_tpset.insertUTF8String(m_cursor, 0, utf8Str);
    }

    if(m_onChange){
        m_onChange(m_tpset.getRawString());
    }
}
