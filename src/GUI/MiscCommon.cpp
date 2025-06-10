#include "../../include/Config/Deckastore.hpp"
#include "../../include/GUI/GUI.hpp"

void gui_init_context() {
    Deckastore& dxstore  = Deckastore::get();
    const DxWindow& dxwindow = dxstore.get_window();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    ImGui::StyleColorsDark();

    gui_set_colors();
    gui_set_style();

    constexpr ImWchar ranges[] = {0x0001, 0xFFFF, 0};
    io.Fonts->AddFontFromMemoryCompressedTTF(OpenSans_compressed_data, OpenSans_compressed_size, 20, nullptr, ranges);
    // TODO:
    //ImFont* header = io.Fonts->AddFontFromMemoryCompressedTTF(OpenSans_compressed_data, OpenSans_compressed_size, 32, nullptr, ranges);
    io.Fonts->Build();

    ImGui_ImplGlfw_InitForOpenGL(dxstore.get_window().get(), true);
    ImGui_ImplOpenGL3_Init();

    dxwindow.register_resize_callback(glfwWindowSizeCallback);
    if (dxstore.get_mode() == Deckadence::mode_t::SERVER)
        accomodate_window_size(dxwindow.get());
}

#ifdef _DEBUG
bool show_demo_window = false;
void gui_draw_performance() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Begin("Performance metrics", nullptr, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize);
    ImGui::Checkbox("Demo  ", &show_demo_window);
    if (show_demo_window) {
        ImGui::ShowDemoWindow(&show_demo_window);
        ImGui::ShowMetricsWindow(&show_demo_window);
    }

    ImGui::SameLine();
    ImGui::Text("Average %.3f ms/f (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
    ImGui::End();
}
#endif
