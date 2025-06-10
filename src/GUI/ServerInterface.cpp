#include "../../include/GUI/GUI.hpp"
#include "../../include/Client/Client.hpp"
#include "../../include/Config/Config.hpp"
#include "../../include/Config/Deckastore.hpp"
#include "../../include/Utilities/SingleInstanceSocket.hpp"

#include <thread>
#include <string>

using std::string;
using std::thread;

// TODO: Add to client
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
        ImGui::Text("Server port: ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(256);
        ImGui::InputInt("##port", &port);
        if (port > 65535 || port < 0)
            port = dxstore.get_port();
        ImGui::EndDisabled();
        ImGui::RadioButton("Client", &mode, 1);
        ImGui::SetCursorPos(ImVec2(340, 326));
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
                config["server"]["discoverable"] = false;
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

void draw_top_bar() {
    static Deckastore& dxstore = Deckastore::get();
    const static uint64_t& selected_uuid = dxstore.get_selected_uuid();
    const static unordered_map<uint64_t, Client>& clients = dxstore.get_client_map();

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.12f, 0.12f, 0.12f, 0.5f));
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGuiWindowFlags iwf = ImGuiWindowFlags_NoCollapse |
                           ImGuiWindowFlags_NoDecoration |
                           ImGuiWindowFlags_NoMove |
                           ImGuiWindowFlags_NoResize |
                           ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 48.0f));
    ImGui::Begin("##top_bar", nullptr, iwf);
    ImGui::SetCursorPos(ImVec2(11, 11));
    if (ImGui::Button("Settings")) {
        dxstore.draw_settings = true;
    }
    ImGui::SetCursorPos(ImVec2(center.x - 128, 14));
    ImGui::Text("Clients");
    ImGui::SameLine();
    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX()+8, 11));
    ImGui::SetNextItemWidth(225);
    if (ImGui::BeginCombo("##clients", clients.empty() ? "<none>" : dxstore.retrieve_current_client().get_nickname().empty() ? std::to_string(selected_uuid).c_str() : dxstore.retrieve_current_client().get_nickname().c_str())) {
        for (auto& it : clients) {
            string label = it.second.get_nickname().empty() ? std::to_string(it.first) : it.second.get_nickname();
            const bool selected = (selected_uuid == it.first);
            if (ImGui::Selectable(label.c_str(), selected)) {
                dxstore.set_selected_uuid(it.first);
                window_resized = true;
            }
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    ImGui::BeginDisabled(!selected_uuid);
    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX()+4, 11));
    // We love ungraceful termination!
    if (ImGui::Button("Properties"))
        dxstore.draw_properties = true;
    ImGui::SameLine();
    ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX()+4, 11));
    if (ImGui::Button("Disconnect"))
        dxstore.disconnect_client(selected_uuid, "Manual disconnect issued.");
    ImGui::EndDisabled();
    ImGui::End();
    ImGui::PopStyleColor();
}

void draw_editor() {
    static Deckastore& dxstore = Deckastore::get();
    const static uint64_t& selected_uuid = dxstore.get_selected_uuid();
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags iwf = ImGuiWindowFlags_NoCollapse |
                           ImGuiWindowFlags_NoDecoration |
                           ImGuiWindowFlags_NoMove |
                           ImGuiWindowFlags_NoResize |
                           ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::SetNextWindowPos(ImVec2(0.0f, 48.0f));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y - 48));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.2f, 0.2f, 0.2f));
    ImGui::Begin("##editor_buttons", nullptr, iwf);
    int rows = 4;
    int columns = 6;
    if (selected_uuid) {
        const Client& cref = dxstore.retrieve_current_client();
        const Profile& pref = cref.profiles[cref.current_profile];
        rows = pref.rows;
        columns = pref.columns;
    }
    ImGui::BeginDisabled(!selected_uuid);
    for (int i = 0; i < rows*columns; ++i) {
        ImGui::PushID(i);
        if (ImGui::Button(!selected_uuid ? "<empty>" :
                          dxstore.retrieve_current_client().get_current_profile_ref().items.at(i).label.c_str(),
                          ImVec2(std::floor(io.DisplaySize.x / columns), std::floor((io.DisplaySize.y - 48) / rows)))) {
            dxstore.draw_item_properties = i;
        }
        ImGui::PopID();
        if ((i+1) % columns)
            ImGui::SameLine();
    }
    ImGui::EndDisabled();
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

int server_gui_init() {
    Deckastore& dxstore = Deckastore::get();
    const status_t& dxstatus = dxstore.get_status();
    // TODO: Add headless mode
    if (dxstore.create_window("Deckadence", {1080, 720})) {
        dxstore.set_status(status_t::DONE);
        return -1;
    }
    const DxWindow& dxwindow = dxstore.get_window();

    gui_init_context();

    ImGuiIO& io = ImGui::GetIO();
    while (dxstatus != status_t::DONE && dxstatus != status_t::RESTARTING) {
        glfwWaitEventsTimeout(max_wait_time);
        if (glfwGetWindowAttrib(dxwindow.get(), GLFW_ICONIFIED)) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        if (window_resized) {
            accomodate_window_size(dxwindow.get());
            window_resized = false;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (dxstore.draw_properties)
            dxstore.retrieve_current_client().draw_properties();
        if (dxstore.draw_settings)
            gui_draw_settings();
        if (dxstore.draw_item_properties != -1)
            dxstore.retrieve_current_client().get_current_profile_ref().items[dxstore.draw_item_properties].draw_properties();
        if (dxwindow.should_close())
            gui_close_dialog(dxwindow.get());
        if (dxstatus == status_t::EXITING || dxstatus == status_t::RESTART)
            gui_show_waiting_tasks();

        draw_top_bar();
        draw_editor();

#ifdef _DEBUG
        gui_draw_performance();
#endif

        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(dxwindow.get());
    };
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    dxstore.destroy_window();
    glfwTerminate();
    return 0;
}
