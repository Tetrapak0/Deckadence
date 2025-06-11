#include "../../include/GUI/GUI.hpp"
#include "../../include/Config/Config.hpp"
#include "../../include/Config/Deckastore.hpp"

void gui_show_waiting_tasks() {
    static Deckastore& dxstore = Deckastore::get();
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
        if (!dxstore.get_client_map().empty() && dxstore.get_mode() == Deckadence::mode_t::SERVER) {
            if (dxstore.get_client_map().size() == 1)
                ImGui::Text("Waiting to disconnect 1 client%s", dotdotdot.c_str());
            else
                ImGui::Text("Waiting to disconnect %llu clients", dxstore.get_client_map().size());
        }
        const static tasks& running_tasks = dxstore.get_tasks();
        if ((running_tasks & tasks::SERVER) != tasks::NONE && dxstore.get_mode() == Deckadence::mode_t::SERVER)
            ImGui::Text("Waiting for server%s", dotdotdot.c_str());
        if ((running_tasks & tasks::CLIENT) != tasks::NONE && dxstore.get_mode() == Deckadence::mode_t::CLIENT)
            ImGui::Text("Waiting for client%s", dotdotdot.c_str());
        if ((running_tasks & tasks::DISCOVERY) != tasks::NONE)
            ImGui::Text("Waiting for discovery service%s", dotdotdot.c_str());
        if ((running_tasks & tasks::SIS) != tasks::NONE && dxstore.get_status() != status_t::RESTART)
            ImGui::Text("Waiting for SIS service%s", dotdotdot.c_str());
        ImGui::EndPopup();
    }
    --ddd_counter;
    if (dxstore.get_client_map().empty() || dxstore.get_mode() == Deckadence::mode_t::CLIENT) {
        if (dxstore.get_tasks() == tasks::NONE && dxstore.get_status() == status_t::EXITING) {
            ddd_counter = 10;
            dotdotdot = "";
            dxstore.set_status(status_t::DONE);
        } else if (dxstore.get_status() == status_t::RESTART &&
                   (dxstore.get_tasks() == tasks::SIS || (!dxstore.has_sis() && dxstore.get_tasks() == tasks::NONE))) {
            ddd_counter = 10;
            dotdotdot = "";
            dxstore.set_status(status_t::RESTARTING);
        }
    }
}

void gui_close_dialog(GLFWwindow* window) {
    static Deckastore& dxstore = Deckastore::get();
    if (dxstore.get_status() != status_t::RUNNING) {
        glfwSetWindowShouldClose(window, GLFW_FALSE);
        return;
    }
    ImGui::OpenPopup("Close Deckadence");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(240, 180));
    if (ImGui::BeginPopupModal("Close Deckadence", nullptr, modalflags)) {
        if (dxstore.get_mode() == Deckadence::mode_t::SERVER) {
            static int close_type = 0;
            ImGui::RadioButton("Hide Deckadence", &close_type, 0);
            ImGui::RadioButton("Exit Deckadence", &close_type, 1);
            ImGui::RadioButton("Restart Deckadence", &close_type, 2);
            // static bool dontaskagain = true;
            // ImGui::Checkbox("Don't ask again", &dontaskagain);
            ImGui::SetCursorPos(ImVec2(100,146));
            if (ImGui::Button("Cancel", ImVec2(64, 26))) {
                glfwSetWindowShouldClose(window, GLFW_FALSE);
            }
            ImGui::SameLine();
            ImGui::SetCursorPosX(168);
            if (ImGui::Button("Confirm")) {
                if (!close_type) {
                    glfwHideWindow(window);
                } else if (close_type == 1) {
                    dxstore.set_status(status_t::EXITING);
                } else if (close_type == 2) {
                    dxstore.set_status(status_t::RESTART);
                }
                close_type = 0;
                glfwSetWindowShouldClose(window, GLFW_FALSE);
            }
        } else {
            dxstore.set_status(status_t::EXITING);
            glfwSetWindowShouldClose(window, GLFW_FALSE);
        }
        ImGui::EndPopup();
    }
}
