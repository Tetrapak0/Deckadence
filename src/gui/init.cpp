#include "../../include/Header.hpp"
#include "../../include/GUI/GUI.hpp"

bool done = false;

string g_shell_uuid = "";

ImGuiWindowFlags g_im_win_flags = ImGuiWindowFlags_NoDecoration
                                | ImGuiWindowFlags_NoBringToFrontOnFocus
                                | ImGuiWindowFlags_AlwaysAutoResize
                                | ImGuiWindowFlags_NoResize
                                | ImGuiWindowFlags_NoMove;

namespace NxSh::GUI {

void gui_get_version() {
    IMGUI_CHECKVERSION();

    LOG(stdout, "Compiled with Dear ImGui ver. %dn", 
           IMGUI_VERSION_NUM);
}

int gui_init(int flags) {
    if (!glfwInit()) {
        fprintf(stderr, "GLFW failed to initialize.\n");
        return -1;
    }

    // if (flags & GF_HIDDEN) glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "NexusShell", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glfwSwapBuffers(window);
#ifdef _WIN32
    {
        HWND hwin = glfwGetWin32Window(window);
        BOOL dark = TRUE;
        DwmSetWindowAttribute(hwin, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));
    }
#endif
    glfwSetWindowSizeLimits(window, 800, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);

    if (flags & GF_HIDDEN) glfwHideWindow(window);

    gui_get_version();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; // Disable imgui.ini
    io.LogFilename = nullptr; //

    ImGui::StyleColorsDark(); 
    ImVec4*     colors = set_colors();
    ImGuiStyle& style  = set_style();

    ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder rgb;
    rgb.AddRanges(io.Fonts->GetGlyphRangesDefault());
    rgb.AddRanges(io.Fonts->GetGlyphRangesCyrillic());
    rgb.AddRanges(io.Fonts->GetGlyphRangesGreek());
    rgb.AddRanges(io.Fonts->GetGlyphRangesJapanese());
    rgb.AddRanges(io.Fonts->GetGlyphRangesKorean());
    rgb.AddRanges(io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    rgb.AddRanges(io.Fonts->GetGlyphRangesChineseFull());
    rgb.AddRanges(io.Fonts->GetGlyphRangesThai());
    rgb.AddRanges(io.Fonts->GetGlyphRangesVietnamese());
    rgb.BuildRanges(&ranges);

    //vector<ImWchar> glyph_ranges = gui_get_glyph_ranges();
    io.Fonts->AddFontFromMemoryCompressedBase85TTF(NotoSans_compressed_data_base85, 20, NULL, ranges.Data);
    io.Fonts->Build();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    do {
        glfwWaitEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        draw_main();
        
#ifdef _DEBUG
        draw_performance();
#endif
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        // glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    } while (!done);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

//  URGENT: FREE ALL TEXTURES BEFORE QUITTING OR CLEARING MAP. DON'T USE exit(-1); IMPLEMENR EXIT FUNCTION

}