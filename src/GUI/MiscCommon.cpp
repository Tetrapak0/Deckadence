#include "Config/Deckastore.hpp"
#include "Config/Config.hpp"
#include "GUI/GUI.hpp"

void gui_overlay_texture(const ImVec2& max_size, const shared_ptr<Texture>& texture) {
        ImVec2 button_min = ImGui::GetItemRectMin();

        float img_w = static_cast<float>(texture->get_width());
        float img_h = static_cast<float>(texture->get_height());
        float btn_w = max_size.x;
        float btn_h = max_size.y;

        float img_aspect = img_w / img_h;
        float btn_aspect = btn_w / btn_h;

        ImVec2 draw_size;
        if (img_aspect > btn_aspect) {
            draw_size.x = btn_w;
            draw_size.y = btn_w / img_aspect;
        } else {
            draw_size.y = btn_h;
            draw_size.x = btn_h * img_aspect;
        }

        ImVec2 padding((btn_w - draw_size.x) * 0.5f, (btn_h - draw_size.y) * 0.5f);

        ImGui::GetWindowDrawList()->AddImage(*texture->get(),
                                             ImVec2(button_min.x + padding.x,
                                                    button_min.y + padding.y),
                                             ImVec2(button_min.x + padding.x + draw_size.x,
                                                    button_min.y + padding.y + draw_size.y));
}

void gui_draw_settings() {
    static Deckastore& dxstore = Deckastore::get();
    static int mode = static_cast<int>(dxstore.get_mode());
    static bool discoverable = dxstore.get_discoverable();
    static int port = dxstore.get_port();
    if (!ImGui::IsPopupOpen("Settings"))
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
        static constexpr unsigned short step = 1;
        static constexpr unsigned short step_fast = 100;
        ImGui::InputScalar("##port", ImGuiDataType_U16, &port, &step, &step_fast);
        ImGui::EndDisabled();
        ImGui::RadioButton("Client", &mode, 1);
        ImGui::SetCursorPos(ImVec2(340, 326));
        if (ImGui::Button("Cancel", ImVec2(64, 26))) {
            mode = 0;
            discoverable = dxstore.get_discoverable();
            port = dxstore.get_port();
            ImGui::CloseCurrentPopup();
            dxstore.draw_settings = false;
        }
        ImGui::SameLine();
        if (mode == static_cast<int>(dxstore.get_mode()) && port == dxstore.get_port()) {
            ImGui::BeginDisabled(discoverable == dxstore.get_discoverable());
            if (ImGui::Button("Apply", ImVec2(64, 26))) {
                json& config = dxstore.get_config();
                dxstore.set_discoverable(discoverable);
                fs::path dkd_dir = get_cfg_dir();
                fs::create_directories(dkd_dir);
                config["server"]["discoverable"] = discoverable;
                // TODO: Deckastore::update_config()
                std::ofstream writer(dkd_dir / "config.json");
                writer << config.dump(4);
                writer.close();
                ImGui::CloseCurrentPopup();
                dxstore.draw_settings = false;
            }
            ImGui::EndDisabled();
        } else if (ImGui::Button("Restart", ImVec2(64, 26))) {
            json& config = dxstore.get_config();
            config["mode"] = mode;
            config["server"]["discoverable"] = discoverable;
            config["server"]["port"] = port;
            fs::path dkd_dir = get_cfg_dir();
            fs::create_directories(dkd_dir);
            std::ofstream writer(dkd_dir / "config.json");
            writer << config.dump(4);
            writer.close();
            ImGui::CloseCurrentPopup();
            dxstore.set_status(status_t::Restart);
            dxstore.draw_settings = false;
        }
        ImGui::EndPopup();
    }
}

void gui_init_context(GLFWwindowposfun move_callback, GLFWwindowsizefun resize_callback) {
    Deckastore& dxstore  = Deckastore::get();
    const DxWindow& dxwindow = dxstore.window;
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    ImGui::StyleColorsDark();

    gui_set_colors();
    gui_set_style();

    // NOTE: There's currently no Asian language support because
    //       the glyphs use too much memory when added this way.
    constexpr ImWchar ranges[] = {0x0001, 0xFFFF, 0};
    dxstore.regular = io.Fonts->AddFontFromMemoryCompressedTTF(OpenSans_compressed_data, OpenSans_compressed_size, 20, nullptr, ranges);
    dxstore.header  = io.Fonts->AddFontFromMemoryCompressedTTF(OpenSans_compressed_data, OpenSans_compressed_size, 36, nullptr, ranges);
    io.Fonts->Build();

    ImGui_ImplGlfw_InitForOpenGL(dxstore.window.get(), true);
    ImGui_ImplOpenGL3_Init();

    if (move_callback)
        dxwindow.register_move_callback(move_callback);
    if (resize_callback)
        dxwindow.register_resize_callback(resize_callback);
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
