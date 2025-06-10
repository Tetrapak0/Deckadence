#pragma once

#include "../../external/imgui/imgui.h"
// #include "../../external/imgui/imgui_impl_sdl3.h"
// #include "../../external/imgui/imgui_impl_sdlrenderer3.h"
// #include "../../external/SDL3/include/SDL.h"

// #if defined(IMGUI_IMPL_OPENGL_ES2)
// #include "../../external/SDL3/include/SDL_opengles2.h"
// #else
// #include "../../external/SDL3/include/SDL_opengl.h"
// #endif

// #include "../../external/Roboto/Roboto.h"

#include <string>

extern bool disconnected_modal;

enum class screens {Home,/*, Vol, Media, Spotify */};

extern int gui_init();

extern ImVec4*     set_colors();
extern ImGuiStyle& set_style();

extern void draw_main(screens current);

extern void draw_home();
extern void draw_killed();
extern void draw_setup();
extern void draw_disconnected_alert();

extern void error_dialog(int type, std::string message);

extern screens get_default_screen();

#ifdef _DEBUG
void draw_performance();
#endif
