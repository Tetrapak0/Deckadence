#pragma once

#ifdef _WIN32
 #pragma comment(linker, "/IMPLIB:NUL /NOIMPLIB /NOEXP")

 #define WIN32_LEAN_AND_MEAN
 #define _CRT_SECURE_NO_WARNINGS 1

 #pragma comment(lib, "opengl32.lib")
 #pragma comment(lib, "../../external/GLFW/lib/glfw3.lib")
 #pragma comment(lib, "../../external/FreeType/bin/freetype.lib")

 #pragma comment(lib, "dwmapi.lib")
 #include <dwmapi.h>
#endif

#include "../../external/imgui/imgui.h"
#include "../../external/imgui/imgui_stdlib.h"
#include "../../external/imgui/imgui_impl_glfw.h"
#include "../../external/imgui/imgui_impl_opengl3.h"
#include "../../external/GLFW/include/glfw3.h"
#ifdef _WIN32
 #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include "../../external/GLFW/include/glfw3native.h"

#include "../../external/NativeFileDIalogs-Extended/include/nfd.hpp"

#include "../../external/OpenSans/OpenSans.h"

#include <vector>
#include <variant>
#include <memory>
#include <thread>
#include <stdexcept>

using std::string;
using std::vector;
using std::variant;
using std::thread;
using std::make_shared;
using std::shared_ptr;
using std::unique_ptr;

extern bool g_draw_shell_props;
extern bool g_draw_button_props;
extern bool g_clr_dialog_shown;

extern int g_button_prop_idx;

extern string g_shell_uuid;

// for functions that don't directly return a status integer,
//      but set it using a thread;
typedef void vint; // TODO: Move to header.h

// TODO: encapsulation

namespace NxSh {
    string open_file(int type);
    namespace GUI {
        enum _flags {
            GF_NONE = 0,        // Hardcoded behaviour (0)
            GF_HIDDEN = 1<<1,   // Hide window on startup (2)
        };

        extern int init(int flags);

        extern void draw_main();
        extern void error_dialog(int type, string message);

        extern ImVec4*	   set_colors();
        extern ImGuiStyle& set_style();

        extern const vector<ImWchar> gui_get_glyph_ranges();

        extern int gui_load_image(const char* filename, GLuint* tptr, int w, int h, bool resize = true);
        extern int systray_init(uint32_t flags);

        #ifdef _DEBUG
        extern void draw_performance();
        #endif
    }
}