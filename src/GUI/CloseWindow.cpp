#include "Config/Deckastore.hpp"
#include "GUI/GUI.hpp"
#include "Config/Config.hpp"

void gui_show_waiting_tasks() {
    static Deckastore& dxstore = Deckastore::get();
    if (!ImGui::IsPopupOpen("Exiting Deckadence"))
        ImGui::OpenPopup("Exiting Deckadence");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(360, 240));
    static int ddd_counter = 10;
    static string dotdotdot;
    if (!ddd_counter) {
        ddd_counter = 10;
        if (dotdotdot == "...")
            dotdotdot = "";
        else
            dotdotdot += ".";
    }
    if (ImGui::BeginPopupModal("Exiting Deckadence", nullptr, modalflags)) {
        if (!dxstore.get_client_map().empty() && dxstore.get_mode() == Deckadence::mode_t::Server) {
            if (dxstore.get_client_map().size() == 1)
                ImGui::Text("Waiting to disconnect 1 client%s", dotdotdot.c_str());
            else
                ImGui::Text("Waiting to disconnect %llu clients", dxstore.get_client_map().size());
        }
        const static tasks& running_tasks = dxstore.get_tasks();
        if ((running_tasks & tasks::Server) != tasks::None && dxstore.get_mode() == Deckadence::mode_t::Server)
            ImGui::Text("Waiting for server%s", dotdotdot.c_str());
        if ((running_tasks & tasks::Client) != tasks::None && dxstore.get_mode() == Deckadence::mode_t::Client)
            ImGui::Text("Waiting for client%s", dotdotdot.c_str());
        if ((running_tasks & tasks::Discovery) != tasks::None)
            ImGui::Text("Waiting for discovery service%s", dotdotdot.c_str());
        if ((running_tasks & tasks::SingleInstance) != tasks::None && dxstore.get_status() != status_t::Restart)
            ImGui::Text("Waiting for SIS service%s", dotdotdot.c_str());
        ImGui::EndPopup();
    }
    --ddd_counter;
    if (dxstore.get_client_map().empty() || dxstore.get_mode() == Deckadence::mode_t::Client) {
        if (dxstore.get_tasks() == tasks::None && dxstore.get_status() == status_t::Exiting) {
            ddd_counter = 10;
            dotdotdot = "";
            dxstore.set_status(status_t::Done);
            ImGui::CloseCurrentPopup();
        } else if (dxstore.get_status() == status_t::Restart &&
                   (dxstore.get_tasks() == tasks::SingleInstance || (!dxstore.IsSingleInstanceEnforced() && dxstore.get_tasks() == tasks::None))) {
            ddd_counter = 10;
            dotdotdot = "";
            dxstore.set_status(status_t::Restarting);
            ImGui::CloseCurrentPopup();
        }
    }
}

void gui_close_dialog() {
    static Deckastore& dxstore = Deckastore::get();
    static DxWindow& dxwindow = dxstore.window;
    if (dxstore.get_status() != status_t::Running) {
        glfwSetWindowShouldClose(dxwindow.get(), GLFW_FALSE);
        return;
    }
    if (!ImGui::IsPopupOpen("Close Deckadence"))
        ImGui::OpenPopup("Close Deckadence");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(240, 180));
    if (ImGui::BeginPopupModal("Close Deckadence", nullptr, modalflags)) {
        if (dxstore.get_mode() == Deckadence::mode_t::Server) {
            static int close_type = 0;
            ImGui::RadioButton("Hide Deckadence", &close_type, 0);
            ImGui::RadioButton("Exit Deckadence", &close_type, 1);
            ImGui::RadioButton("Restart Deckadence", &close_type, 2);
            // static bool dontaskagain = true;
            // ImGui::Checkbox("Don't ask again", &dontaskagain);
            ImGui::SetCursorPos(ImVec2(100, 146));
            if (ImGui::Button("Cancel", ImVec2(64, 26))) {
                glfwSetWindowShouldClose(dxwindow.get(), GLFW_FALSE);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Confirm")) {
                if (!close_type) {
                    dxstore.window.hide();
                } else if (close_type == 1) {
                    dxstore.set_status(status_t::Exiting);
                } else if (close_type == 2) {
                    dxstore.set_status(status_t::Restart);
                    close_type = 0;
                }
                glfwSetWindowShouldClose(dxwindow.get(), GLFW_FALSE);
                ImGui::CloseCurrentPopup();
            }
        } else {
            dxstore.set_status(status_t::Exiting);
            glfwSetWindowShouldClose(dxwindow.get(), GLFW_FALSE);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}
