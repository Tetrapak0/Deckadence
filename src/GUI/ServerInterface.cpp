#include "../../include/Config/Deckastore.hpp"
#include "../../include/GUI/GUI.hpp"
#include "../../include/Client/Client.hpp"
#include "../../include/Config/Config.hpp"
#include "../../include/Utilities/SingleInstanceSocket.hpp"

#include <thread>
#include <string>

using std::string;
using std::thread;

void gui_draw_item_properties() {
    Deckastore& dxstore = Deckastore::get();
    Client& dxclient = dxstore.retrieve_current_client();
    ItemTypeRegistry& dxtypereg = dxstore.type_registry;
    ImGui::OpenPopup("Button properties");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(640, 480));
    static bool button_cleared = false;
    static shared_ptr<Item> tmp = nullptr;
    dxstore.lock.lock();
    Profile& dxprofile = dxclient.get_current_profile_ref();
    if (!tmp)
        tmp = dxprofile.root->items[dxstore.draw_item_properties];
    if (ImGui::BeginPopupModal("Button properties", nullptr, modalflags)) {
        ImGui::BeginDisabled();
        ImGui::TextWrapped("(i) Please do not use environment variables. Write your full user path (if applicable), not ~/");
        ImGui::EndDisabled();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Type: ");
        ImGui::SameLine();
        if (ImGui::BeginCombo("##Type", tmp->get_typename())) {
            for (const auto& [fst, snd] : dxtypereg.get_registry()) {
                const bool selected = string(tmp->get_typename()) == fst;
                ImGui::BeginDisabled(fst == "Go up" && !dxprofile.root->parent);
                if (ImGui::Selectable(fst.c_str(), selected)) {
                    string label = tmp->m_label;
                    if (fst != tmp->get_typename()) {
                        if (string(dxprofile.root->items[dxstore.draw_item_properties]->get_typename()) == fst) {
                            tmp = dxprofile.root->items[dxstore.draw_item_properties];
                            dxprofile.root->items[dxstore.draw_item_properties]->m_label = label;
                        } else {
                            tmp = std::shared_ptr<Item>(snd(nullptr, dxprofile, dxprofile.root));
                            tmp->m_label = label;
                        }
                    }
                }
                if (selected)
                    ImGui::SetItemDefaultFocus();
                ImGui::EndDisabled();
            }
            ImGui::EndCombo();
        }

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Label: ");
        ImGui::SameLine();
        // TODO: Multiline label support
        ImGui::InputText("##lbl", &tmp->m_label);

        tmp->draw_properties();

        // TODO: Move these to a child window so it doesn't clash with draw_properties if that's too long
        // ImGui::SetCursorPos(ImVec2(340, 326));
        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth()-140-68, ImGui::GetWindowHeight()-34));
        if (ImGui::Button("Clear", ImVec2(64, 24))) {
            tmp = std::make_shared<ExecutableItem>(ExecutableItem(nullptr, dxprofile, dxprofile.root));
            button_cleared = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(64, 26))) {
            dxprofile.root->items[dxstore.draw_item_properties]->properties_cancel();
            tmp = nullptr;
            dxstore.draw_item_properties = -1;
            button_cleared = false;
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(button_cleared ? false : tmp ? tmp->prop_apply_disable() : true);
        if (ImGui::Button("Apply", ImVec2(64, 26))) {
            if (button_cleared) {
                json* cfg = dxprofile.root->items[dxstore.draw_item_properties]->config;
                json& items = (*dxprofile.root->config)["items"];
                if (cfg) {
                    items.erase(std::remove_if(items.begin(), items.end(), [&](const json& item) {
                        // idk maybe no index means the user made a temporary entry
                        return cfg->contains("idx") && item.contains("idx") ? item["idx"] == (*cfg)["idx"] : false;
                    }), items.end());
                    dxclient.update_config();
                }
            } else {
                tmp->properties_apply();
            }
            dxprofile.root->items[dxstore.draw_item_properties] = tmp;
            tmp = nullptr;
            dxstore.draw_item_properties = -1;
            button_cleared = false;
        }
        dxstore.lock.unlock();
        ImGui::EndDisabled();
        ImGui::EndPopup();
    }
}

void draw_top_bar() {
    static Deckastore& dxstore = Deckastore::get();
    const static uint64_t& selected_uuid = dxstore.get_selected_uuid();
    const static unordered_map<uint64_t, Client>& clients = dxstore.get_client_map();
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.05f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(11, 11));
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
    if (ImGui::Button("Settings")) {
        dxstore.draw_settings = true;
    }
    ImGui::SameLine();
    // TODO: Add help button
    ImGui::SetCursorPosX(center.x - 128);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("Clients");
    ImGui::SameLine();
    // ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX()+8, 11));
    ImGui::SetNextItemWidth(225);
    dxstore.lock.lock();
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
    if (ImGui::Button("Properties"))
        dxstore.draw_properties = true;
    ImGui::SameLine();
    dxstore.lock.unlock();
    if (ImGui::Button("Disconnect"))
        dxstore.disconnect_client(selected_uuid, "Manual disconnect issued.");
    // TODO: Use message on client
    // TODO: Add lock to client when server's session is locked (toggleable)
    ImGui::EndDisabled();
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
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
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.2f, 0.2f, 0.2f));
    ImGui::Begin("##editor_buttons", nullptr, iwf);
    int rows = 4;
    int columns = 6;
    dxstore.lock.lock();
    if (selected_uuid) {
        const Client& cref = dxstore.retrieve_current_client();
        const Profile& pref = *cref.profiles[cref.current_profile];
        rows = pref.rows;
        columns = pref.columns;
    }
    ImGui::BeginDisabled(!selected_uuid);
    for (int i = 0; i < rows*columns; ++i) {
        ImGui::PushID(i);
        bool disabled = selected_uuid ? dxstore.retrieve_current_client().get_current_profile_ref().root->items[i]->prop_apply_disable() : false;
        if (disabled) {
            if (dxstore.retrieve_current_client().get_current_profile_ref().root->items[i]->label.empty())
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2, 0, 0, 0.54));
        }
        ImGui::Button(!selected_uuid ? "<empty>" :
                      dxstore.retrieve_current_client().get_current_profile_ref().root->items[i]->label.c_str(),
                    ImVec2(std::floor(io.DisplaySize.x / columns), std::floor((io.DisplaySize.y - 48) / rows)));
        if (ImGui::IsItemHovered()) {
            // TODO: We want double click to open instead
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                Profile& dxprofile = dxstore.retrieve_current_client().get_current_profile_ref();
                string msg = string(1, DX_EXECUTE_BYTE) + std::to_string(i);
                // TODO: Change to something like Item::openable_in_editor
                if (!strcmp(dxprofile.root->items[i]->get_typename(), "Folder")) {
                    dxprofile.parent.nav_history.emplace_back(msg);
                    dxprofile.root->items[i]->execute();
                    send(dxprofile.parent.socket, msg.c_str(), msg.length(), 0);
                } else if (!strcmp(dxprofile.root->items[i]->get_typename(), "Go up")) {
                    if (dxprofile.root->parent) {
                        dxprofile.parent.nav_history.pop_back();
                        dxprofile.root->items[i]->execute();
                        send(dxprofile.parent.socket, msg.c_str(), msg.length(), 0);
                    }
                }
            } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                dxstore.draw_item_properties = i;
            }
        }
        ImGui::PopID();
        if ((i+1) % columns)
            ImGui::SameLine();
        if (disabled) {
            ImGui::PopStyleColor();
        }
    }
    dxstore.lock.unlock();
    ImGui::EndDisabled();
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(4);
}

int server_gui_init() {
    Deckastore& dxstore = Deckastore::get();
    const status_t& dxstatus = dxstore.get_status();
    // TODO: Add headless mode
    if (dxstore.create_window("Deckadence", {1080, 720})) {
        dxstore.set_status(status_t::DONE);
        return -1;
    }
    const DxWindow& dxwindow = dxstore.window;

    gui_init_context();

    ImGuiIO& io = ImGui::GetIO();
    while (dxstatus != status_t::DONE && dxstatus != status_t::RESTARTING) {
        glfwWaitEventsTimeout(max_wait_time);
        if (glfwGetWindowAttrib(dxwindow.get(), GLFW_ICONIFIED)) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        if (window_resized) {
            accommodate_window_size();
            window_resized = false;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (dxwindow.should_close()) {
            gui_close_dialog();
        } else if (dxstatus == status_t::EXITING || dxstatus == status_t::RESTART) {
            gui_show_waiting_tasks();
        } else {
            if (dxstore.draw_item_properties != -1) {
                gui_draw_item_properties();
            } else if (dxstore.draw_properties) {
                dxstore.retrieve_current_client().draw_properties();
            } else if (dxstore.draw_settings) {
                gui_draw_settings();
            }
        }

        draw_top_bar();
        draw_editor();

// #ifdef _DEBUG
//         gui_draw_performance();
// #endif

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
