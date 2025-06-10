#include "../../include/Header.hpp"
#include "../../include/GUI/GUI.hpp"
#include "../../include/Config.hpp"

#define SITUATION_BTN_PROP 0x0000000000000001 // Show item properties
#define SITUATION_SHL_PROP 0x0000000000000010 // Show shell properties
#define SITUATION_SET_SHOW 0x0000000000000100 // Show NxSh settings
#define SITUATION_SET_NULL 0x0000000000001000 // Null out items in item props
#define SITUATION_CLR_SHOW 0x0000000000010000 // Show item clear dialog
#define SITUATION_RST_SERV 0x0000000000100000 // Restart server socket
#define SITUATION_PRG_DONE 0x0000000001000000 // NxSh done and ready to shutdown
#define SITUATION_BTN_OVRW 0x0000000010000000 // Item type conflict dialog shown

int g_situation = 0;

int g_button_prop_idx = -1;

extern ImGuiWindowFlags g_im_win_flags;

namespace NxSh {

static unordered_map<string, std::function<unique_ptr<GUI::Item>()>> typenames;

template <typename T>
void register_type() {
    typenames[T::item_typename] = std::make_unique(T());
}

const unordered_map<string, std::function<unique_ptr<GUI::Item>()>>& get_types() {
    return typenames;
}

// TODO: Return int and let show_item_props take care of clearing
int draw_clear_dialog() {
    ImGui::OpenPopup("Clear Item");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Clear Item", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::Text("Are you sure you want to clear this item?");
        if (ImGui::Button("Yes")) {
            NxSh::clear_button(CURRENT_SHELL.profile_idx, 0, g_button_prop_idx);
            reconfigure_shell(CURRENT_SHELL);
            g_situation |= SITUATION_SET_NULL;
        }
        ImGui::SameLine();
        if (ImGui::Button("No")) g_clr_dialog_shown = false;
        ImGui::EndPopup();
    }
}

int draw_overw_dialog() {
    ImGui::OpenPopup("Type conflict");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Type conflict", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        ImGui::Text("You are trying to create a button of a different type than the current one.\n");
        ImGui::Text("Any custom members will be lost!");
        ImGui::Text("Are you sure you want to continue?");
        if (ImGui::Button("Yes")) return 0;
        ImGui::SameLine();
        if (ImGui::Button("No")) return -1;
        ImGui::EndPopup();
    }
    return 0xF;
}

void draw_item_properties(GUI::Item& item) {
    ImGui::OpenPopup("Button Properties");
    static const char* display_types[] = {"Text", "Image"};
    static string lbtmp;
    static string typtmp;
    static int    usethtmp;
    if (g_situation & SITUATION_SET_NULL) {
        lbtmp        = item.get_label();
        typtmp       = item.item_typename;
        usethtmp     = item.uses_thumbnail;
        g_situation &= ~SITUATION_SET_NULL;
    }
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Button Properties", NULL, ImGuiWindowFlags_NoResize | 
                                                          ImGuiWindowFlags_NoMove)) {
        if (g_situation & SITUATION_CLR_SHOW) {
            switch (draw_clear_dialog()) {
                
            }
        }
        if (g_situation & SITUATION_BTN_OVRW) {
            switch (draw_overw_dialog()) {
                case 0xF: break;
                case 0: {

                } case -1: {
                    g_situation &= ~ SITUATION_BTN_OVRW;
                    break;
                }
            }
        }
        ImGui::SetWindowSize(ImVec2(450, 250));
        ImGui::SeparatorText("Display");
        ImGui::Text("Content:"); ImGui::SameLine();
        if (ImGui::BeginCombo("##Display Type", display_types[usethtmp])) {
            for (int i = 0; i < IM_ARRAYSIZE(display_types); ++i) {
                const bool is_selected = (usethtmp == i);
                if (ImGui::Selectable(display_types[i], is_selected)) usethtmp = i;
                if (is_selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        if (usethtmp) {
            ImGui::Text("Path:"); ImGui::SameLine();
            ImGui::InputText("##Path", &lbtmp); ImGui::SameLine();
            if (ImGui::Button("Browse"))
                lbtmp = NxSh::open_file(1);
        } else {
            ImGui::Text("Label:"); ImGui::SameLine();
            ImGui::InputText("##Label", &lbtmp);
        }
        ImGui::SeparatorText("Behavior");
        ImGui::Text("Type:"); ImGui::SameLine();
        if (ImGui::BeginCombo("##Type", typtmp.c_str())) {
            for (const auto& it : typenames) {
                const bool is_selected = (typtmp == it.first);
                if (ImGui::Selectable(it.first.c_str(), is_selected))
                    typtmp = it.first;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
        }
        auto to = typenames.find(typtmp)->second();

        item.show_properties();

        ImGui::BeginDisabled(usethtmp && lbtmp.empty());
        if (ImGui::Button("OK")) {
            CURRENT_SHELL.lock.lock();
            // TODO: Implement undo
            if (item.item_typename != typtmp) g_situation |= SITUATION_BTN_OVRW;
            bool label_change = false;
            if (lbtmp != item.label) label_change = true;
            // TODO: unordered map btn.vars btn.types for here
            item.uses_thumbnail = usethtmp;
            if (item.uses_thumbnail) {
                if (item.thumbnail.is_uninit() || label_change)
                    item.thumbnail.create(item.label.c_str());
            }
            item.set_label(lbtmp);
            item.set_dtype(usethtmp);
            // TODO: Change page to CURRENT_PROFILE.page_idx
            json& cfg = CURRENT_SHELL.config[g_shell_uuid]["profiles"][to_string(CURRENT_SHELL.profile_idx)]["pages"]["0"]["buttons"][to_string(g_button_prop_idx)];
            cfg["display type"] = to_string(item.uses_thumbnail);
            cfg["label"] = item.label;
            cfg["type"] = item.item_typename;
            config_write(CURRENT_SHELL);
            reconfigure_shell(CURRENT_SHELL);
            CURRENT_SHELL.lock.unlock();
        }
        ImGui::EndDisabled();
        ImGui::SameLine();  // TODO: Move ok/cancel to right
        if (ImGui::Button("Clear")) g_situation |= SITUATION_CLR_SHOW;
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            g_situation |=  SITUATION_SET_NULL;
            g_situation &= ~SITUATION_BTN_PROP;
        }
        ImGui::EndPopup();
    }
}

// needs to be passed by pointer so that nullptr can be passed
//    when the button object cannot be shown or is unavailable
void draw_button(GUI::Item* btn, int index) {
    ImGuiIO& io = ImGui::GetIO();
    float x_size = io.DisplaySize.x - 175;
    if (g_shell_uuid != "" && btn) {
        string lbl = "##";
        if (!btn->get_label().empty())
            lbl = btn->get_label();
        if (ImGui::Button(lbl.c_str(), ImVec2(x_size / CURRENT_PROFILE.columns,
            io.DisplaySize.y / CURRENT_PROFILE.rows))) {
            g_button_prop_idx = index;
            g_draw_button_props = true;
            draw_item_properties(*btn);
        }
    } else ImGui::Button(std::to_string(index).c_str(), ImVec2(x_size / 6, io.DisplaySize.y / 4));
}

void draw_folder(GUI::Folder& fdr, int index) { // There will NEVER be a null folder as buttons are default. References are fine
    ImGuiIO& io = ImGui::GetIO();
    float x_size = io.DisplaySize.x - 175;
    if (g_shell_uuid != "") {
        string lbl = "##";
        if (!fdr.get_label().empty())
            lbl = fdr.get_label();
        if (ImGui::Button(lbl.c_str(), ImVec2(x_size / CURRENT_PROFILE.columns,
            io.DisplaySize.y / CURRENT_PROFILE.rows))) {
            g_button_prop_idx = index;
            g_draw_button_props = true;
            draw_item_properties(fdr);
        }
    } else ImGui::Button(std::to_string(index).c_str(), ImVec2(x_size / 6, io.DisplaySize.y / 4));
}

void draw_editor() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    float x_size = io.DisplaySize.x - 175;
    ImGui::SetNextWindowSize(ImVec2(x_size, io.DisplaySize.y));
    ImGui::SetNextWindowPos(ImVec2(176.0f, 0.0f));
    ImGui::Begin("editor", NULL, g_im_win_flags);
    if (g_shell_uuid != "") {
        CURRENT_SHELL.lock.lock();
        for (int i = 0; i < CURRENT_PROFILE.columns * CURRENT_PROFILE.rows; ++i) {
            if (!(g_situation & SITUATION_SHL_PROP)) {
                // TODO: FIX
                // if (std::holds_alternative<Button>((*CURRENT_PROFILE.editor_items)[i])) { // TODO: Check display type dumbass
                //     // the following reference is created to reduce the amount of std::get calls required.
                //     // Might just create a reference at the start of the loop.
                //     Button& btn = std::get<Button>((*CURRENT_PROFILE.editor_items)[i]);
                //     if (!static_cast<int>(btn.dtype)) {
                //         if (btn.default_label) draw_button(&btn, i);
                //         else if (btn.label.empty()) draw_button(&btn, i);
                //         else draw_button(&btn, i);
                //     } else {
                //         if (ImGui::ImageButton("##", (ImTextureID)(intptr_t)btn.thumbnail.get(), 
                //                                ImVec2(x_size / CURRENT_PROFILE.columns,
                //                                       io.DisplaySize.y / CURRENT_PROFILE.rows))) {
                //             g_button_prop_idx = i;
                //             g_draw_button_props = true;
                //             draw_button_propertiese(btn);
                //         }
                //     }
                // } else if (std::holds_alternative<Folder>((*CURRENT_PROFILE.editor_items)[i])) {
                //     GUI::Folder& fdr = std::get<Folder>((*CURRENT_PROFILE.editor_items)[i]);
                //     if (!fdr.uses_thumbnail) {
                //         if (fdr.default_label) draw_folder(fdr, i);
                //     }
                // }
            } else draw_button(nullptr, i);
            if (i % CURRENT_PROFILE.columns) ImGui::SameLine();
        }
        CURRENT_SHELL.lock.unlock();
    } else {
        ImGui::BeginDisabled();
        for (int i = 0; i < 24; ++i) {
            draw_button(nullptr, i);
            if (i % 6) ImGui::SameLine();
        }
        ImGui::EndDisabled();
    }
    ImGui::PopStyleVar(2);
    if (g_draw_button_props && NxSh::shells.count(g_shell_uuid)) {
        if (CURRENT_SHELL.get_uuid() == g_shell_uuid)
            draw_item_properties((*CURRENT_PROFILE.editor_items)[g_button_prop_idx]);
        else {
            g_draw_button_props = false;
            g_shell_uuid = "";
        }
    }
    ImGui::End();
}

void draw_shell_properties() {
    static int columns, rows;
    static string nickname;
    if (g_situation & SITUATION_SET_NULL) {
        columns = CURRENT_PROFILE.columns;
        rows = CURRENT_PROFILE.rows;
        nickname = CURRENT_SHELL.nickname;
        g_situation &= ~SITUATION_SET_NULL;
    }
    ImGui::OpenPopup("Device Properties");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowSize(ImVec2(450, 250));
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Device Properties", NULL, ImGuiWindowFlags_NoMove |
                                                          ImGuiWindowFlags_NoResize)) {
        ImGui::Text("Properties for "); ImGui::SameLine();
        ImGui::Text("%s", g_shell_uuid.c_str());
        ImGui::SeparatorText("General");
        ImGui::Text("Nickname:"); ImGui::SameLine();
        ImGui::InputText("##nickname", &CURRENT_SHELL.nickname);
        ImGui::SeparatorText("Layout");
        ImGui::Text("Columns:"); ImGui::SameLine(); 
        ImGui::InputInt("##Columns", &CURRENT_PROFILE.columns);
        ImGui::Text("Rows:"); ImGui::SameLine(); 
        ImGui::InputInt("##Rows", &CURRENT_PROFILE.rows);
        if (CURRENT_PROFILE.columns * CURRENT_PROFILE.rows > 400) {
            CURRENT_PROFILE.columns = columns;
            CURRENT_PROFILE.rows = rows;
        }
        if (ImGui::Button("Ok")) {
            CURRENT_SHELL.lock.lock();
            CURRENT_SHELL.config[g_shell_uuid]["nickname"] = CURRENT_SHELL.nickname;
            CURRENT_SHELL.config[g_shell_uuid]["profiles"][to_string(CURRENT_SHELL.profile_idx)]["columns"] = to_string(CURRENT_PROFILE.columns);
            CURRENT_SHELL.config[g_shell_uuid]["profiles"][to_string(CURRENT_SHELL.profile_idx)]["rows"] = to_string(CURRENT_PROFILE.rows);
            config_write(CURRENT_SHELL);
            int new_grid = CURRENT_PROFILE.columns * CURRENT_PROFILE.rows;
            int old_grid = columns * rows;
            if (new_grid > old_grid) {
                int iter_len = new_grid - CURRENT_PROFILE.items->size();
                for (int i = 0; i < iter_len; ++i)
                    CURRENT_PROFILE.items->emplace_back(NxSh::GUI::FileButton());
            } else if (new_grid < old_grid) {
                vector<NxSh::GUI::Item> swapper;
                int button_count = CURRENT_PROFILE.items->size();
                for (int i = 0; i < button_count; ++i) {
                    if (i < new_grid) swapper.push_back((*CURRENT_PROFILE.items)[i]);
                    else NxSh::clear_button(CURRENT_SHELL.profile_idx, 0, i);
                }
                CURRENT_PROFILE.items->clear();
                *CURRENT_PROFILE.items = swapper;
            }
            reconfigure_shell(CURRENT_SHELL);
            CURRENT_SHELL.lock.unlock();
            g_situation |= SITUATION_SET_NULL;
            g_draw_shell_props = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            CURRENT_PROFILE.columns = columns;
            CURRENT_PROFILE.rows    = rows;
            CURRENT_SHELL.nickname  = nickname;
            g_situation |= SITUATION_SET_NULL;
            g_draw_shell_props = false;
        }
        ImGui::EndPopup();
    }
}

void draw_settings() {
    static int temp_port;
    if (g_situation & SITUATION_SET_NULL) {
        temp_port = port;
        g_situation &= ~SITUATION_SET_NULL;
    }
    ImGui::OpenPopup("Settings");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowSize(ImVec2(450, 250));
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Settings", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
        ImGui::Text("Port: "); ImGui::SameLine();
        ImGui::InputInt("##Port", &port);
        if (ImGui::Button("Ok")) {
            string portstore = getenv("USERPROFILE");           // TODO: move to rw_portstore
            portstore += "\\AppData\\Roaming\\NexusShell\\";
            if (!exists(portstore)) create_directory(portstore);
            portstore += "portstore";
            ofstream writer(portstore);
            writer << port;
            writer.close();
            g_situation |= SITUATION_SET_NULL;
            g_situation &= ~SITUATION_SET_SHOW;
            restart_server = true;
            nx_sock_close(listen_socket);
            string message = "killreason: port changed to " + to_string(port);
            for (auto& shell : NxSh::shells) {
                send(shell.second.client, message.c_str(), message.length() + 1, 0);
                nx_sock_close(shell.second.client);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            port = temp_port;
            g_situation |= SITUATION_SET_NULL;
            g_situation &= ~SITUATION_SET_SHOW;
        }
        ImGui::EndPopup();
    }
}

void draw_sock_reboot() {
    ImGui::OpenPopup("Restarting Server...");
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Restarting Server...", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
        ImGui::Text("Please wait while the server is restarted.");
        ImGui::EndPopup();
    }
}

void draw_top_bar() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 24));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::Begin("top_bar", NULL, g_im_win_flags);
    ImGui::Text("Shells");
    ImGui::SameLine();
    if (ImGui::BeginCombo("##shells", g_shell_uuid.empty() ? "-" : g_shell_uuid.c_str())) {
        for (auto& pair : NxSh::shells) {
            const bool is_selected = (g_shell_uuid == pair.first);
            if (ImGui::Selectable(pair.first.c_str(), is_selected))
                g_shell_uuid = pair.first;
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
}

void _draw_main() { // TODO rename
    ImGuiIO& io = ImGui::GetIO();
    if (g_situation & SITUATION_RST_SERV) draw_sock_reboot();
    if (g_situation & SITUATION_SET_SHOW) draw_settings();
    draw_top_bar();
}

void draw_main() {
    ImGuiIO& io = ImGui::GetIO();
    if (restart_server) draw_sock_reboot();
    if (g_situation & SITUATION_SET_SHOW) draw_settings(),
    ImGui::SetNextWindowSize(ImVec2(175, io.DisplaySize.y - 33));
    ImGui::SetNextWindowSizeConstraints(ImVec2(175, 367), ImVec2(175, FLT_MAX));
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::Begin("devices", NULL, g_im_win_flags);
    ImGui::Text("Connected Devices:\n");
    NxSh::shells_lock.lock();
    if (!NxSh::shells.empty()) {
        if (g_draw_shell_props) draw_shell_properties();
        for (unordered_map<string, NxSh::Shell>::iterator it = NxSh::shells.begin(); it != NxSh::shells.end(); ++it) {
            if (it->second.nickname.empty()) {
                if (ImGui::Selectable(it->first.c_str(), g_shell_uuid == it->first))
                    g_shell_uuid = it->first;
            } else {
                if (ImGui::Selectable(it->second.nickname.c_str(), g_shell_uuid == it->first))
                    g_shell_uuid = it->first;
            }
            if (ImGui::BeginPopupContextItem()) {
                g_shell_uuid = it->first;
                if (ImGui::MenuItem("Properties")) g_draw_shell_props = true;
                if (ImGui::MenuItem("Disconnect")) {
                    send(CURRENT_SHELL.client, "killreason: disconnected by user", strlen("killreason: disconnected by user")+1, 0);
                    nx_sock_close(CURRENT_SHELL.client);
                    g_shell_uuid = "";
                }
                ImGui::EndPopup();
            }
        }
    } else { 
        g_shell_uuid = "";
        ImGui::Text("None"); 
    }
    ImGui::End();
    ImGui::SetNextWindowSize(ImVec2(175.0f, 32.0f));
    ImGui::SetNextWindowSizeConstraints(ImVec2(175.0f, 33.0f), ImVec2(175.0f, 33.0f));
    ImGui::SetNextWindowPos(ImVec2(0.0f, io.DisplaySize.y - 32));
    ImGui::Begin("About", NULL, g_im_win_flags);
    if (ImGui::Button("Settings")) g_situation |= SITUATION_SET_NULL;
    ImGui::End();
    draw_editor();
    NxSh::shells_lock.unlock();
}

//#define _DEBUG
#ifdef _DEBUG
bool show_demo_window = false;
void draw_performance() {
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Begin("Performance Statistics", NULL, ImGuiWindowFlags_NoCollapse | 
                                                 ImGuiWindowFlags_NoDocking);
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

}