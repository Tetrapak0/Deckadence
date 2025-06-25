#include "../../include/Config/Deckastore.hpp"
#include "../../include/Config/Config.hpp"
#include "../../include/GUI/GUI.hpp"

void gui_draw_settings() {
    static Deckastore& dxstore = Deckastore::get();
    static int mode = 0;
    static bool discoverable = dxstore.get_discoverable();
    static int port = dxstore.get_port();
    ImGui::OpenPopup("Settings");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(480, 360));
    if (ImGui::BeginPopupModal("Settings", nullptr, modalflags)) {
        ImGui::Text("Should this machine act as a server or client?");
        ImGui::RadioButton("Server", &mode, 0);
        ImGui::BeginDisabled(mode);
        ImGui::SetCursorPosX(24);
        ImGui::Checkbox("Make this server discoverable on my network", &discoverable);
        ImGui::SetCursorPosX(24);
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Server port: ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(256);
        ImGui::InputInt("##port", &port);
        if (port > 65535 || port < 0)
            port = dxstore.get_port();
        ImGui::EndDisabled();
        ImGui::RadioButton("Client", &mode, 1);
        ImGui::SetCursorPos(ImVec2(340, 328));
        if (ImGui::Button("Cancel", ImVec2(64, 24))) {
            dxstore.draw_settings = false;
            mode = 0;
            discoverable = dxstore.get_discoverable();
            port = dxstore.get_port();
        }
        ImGui::SameLine();
        if (!mode && port == dxstore.get_port()) {
            ImGui::BeginDisabled(discoverable == dxstore.get_discoverable());
            if (ImGui::Button("Apply", ImVec2(64, 24))) {
                json& config = dxstore.get_config();
                dxstore.set_discoverable(discoverable);
                fs::path dkd_dir = get_cfg_dir();
                fs::create_directories(dkd_dir);
                config["server"]["discoverable"] = discoverable;
                // TODO: Deckastore::update_config()
                std::ofstream writer(dkd_dir / "config.json");
                writer << config.dump(4);
                writer.close();
                dxstore.draw_settings = false;
            }
            ImGui::EndDisabled();
        } else if (ImGui::Button("Restart", ImVec2(64, 24))) {
            json& config = dxstore.get_config();
            config["mode"] = mode;
            config["server"]["discoverable"] = discoverable;
            config["server"]["port"] = port;
            fs::path dkd_dir = get_cfg_dir();
            fs::create_directories(dkd_dir);
            std::ofstream writer(dkd_dir / "config.json");
            writer << config.dump(4);
            writer.close();
            dxstore.set_status(status_t::RESTART);
            dxstore.draw_settings = false;
        }
        ImGui::EndPopup();
    }
}

void gui_init_context() {
    Deckastore& dxstore  = Deckastore::get();
    const DxWindow& dxwindow = dxstore.window;
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

    ImGui_ImplGlfw_InitForOpenGL(dxstore.window.get(), true);
    ImGui_ImplOpenGL3_Init();

    dxwindow.register_resize_callback(glfwWindowSizeCallback);
    // TODO: Change
    glfwSetWindowPosCallback(dxwindow.get(), glfwWindowSizeCallback);
    if (dxstore.get_mode() == Deckadence::mode_t::SERVER)
        accommodate_window_size();
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
