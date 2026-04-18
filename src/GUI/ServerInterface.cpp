#include "Config/Deckastore.hpp"
#include "GUI/GUI.hpp"
#include "Client/Client.hpp"
#include "Config/Config.hpp"
#include "Config/Item.hpp"
#include "Config/ExecutableItem.hpp"
#include "Utilities/SingleInstanceSocket.hpp"

extern "C" {
    #include "../../resources/EnterIcon.h"
}

#include "../../external/imgui/imgui_internal.h"

#include <thread>
#include <string>

using std::string;
using std::thread;

void accommodate_window_size() {
    Deckastore& dxstore = Deckastore::get();
    DxWindow& dxwindow = dxstore.window;
    static int w_old = 0, h_old = 0;
    if (!w_old || !h_old) {
        Vec2<int> size = dxwindow.get_size();
        w_old = size.x;
        h_old = size.y;
    }
    static int columns = 6, rows = 4;
    if (dxstore.get_selected_uuid()) {
        const Profile& p = dxstore.retrieve_current_client().get_current_profile_ref();
        columns = p.columns;
        rows = p.rows;
    }
    Vec2<int> size = dxwindow.get_size();
    int w = size.x, h = size.y;
    int req_w = 0, req_h = 0;
    int excess_w = w % columns;
    int excess_h = (h - 48) % rows;
    bool resized_too_small = false;
    if (w < columns * 20) {
        resized_too_small = true;
        // printf("w smaller than 20 columns\n");
    }
    if (excess_w > columns / 2 || resized_too_small || w_old < w) {
        req_w = columns - excess_w;
        // printf("w bigger than columns/2\n");
    } else {
        req_w = -excess_w;
        // printf("w smaller than columns/2\n");
    }

    resized_too_small = false;
    if (h < rows * 20) {
        resized_too_small = true;
        // printf("h smaller than 20 rows\n");
    }
    if (excess_h > rows / 2 || resized_too_small || h_old < h) {
        req_h = rows - excess_h;
        // printf("h bigger than rows/2\n");
    } else {
        req_h = -excess_h;
        // printf("h smaller than rows/2\n");
    }
    // printf("width: %d, excess: %d, required: %d\n", w, excess_w, req_w);
    // printf("height: %d, excess: %d, required: %d\n", h, excess_h, req_h);
    dxwindow.resize(Vec2<int>(w + req_w, h + req_h));
    w_old = w;
    h_old = h;
}

void gui_draw_item_properties() {
    Deckastore& dxstore = Deckastore::get();
    dxstore.lock.lock();
    Client& dxclient = dxstore.retrieve_current_client();
    ItemTypeRegistry& dxtypereg = dxstore.type_registry;
    Profile& dxprofile = dxclient.get_current_profile_ref();

    bool reset_tab = false;
    static shared_ptr<Item> tmp = nullptr;
    if (!ImGui::IsPopupOpen("Button properties")) {
        ImGui::OpenPopup("Button properties");
        tmp = dxprofile.root->items[dxstore.draw_item_properties];
        reset_tab = true;
    }
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(640, 480));
    // TODO: Live preview
    if (ImGui::BeginPopupModal("Button properties", nullptr, modalflags)) {
        ImGui::BeginChild("##container", ImVec2(ImGui::GetWindowWidth()-ImGui::GetStyle().WindowPadding.x*2, 480-ImGui::GetFontSize()-ImGui::GetStyle().FramePadding.y*2-42-ImGui::GetStyle().WindowPadding.y));
        if (ImGui::BeginTabBar("##categories", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton)) {
            if (ImGui::BeginTabItem("General", nullptr, reset_tab ? ImGuiTabItemFlags_SetSelected : 0)) {
                ImGui::BeginChild("##cfg", ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()-ImGui::GetFrameHeight()-ImGui::GetStyle().FramePadding.y-1));
                ImGui::AlignTextToFramePadding();
                ImGui::Text("Type: ");
                ImGui::SameLine();
                if (ImGui::BeginCombo("##Type", tmp->get_typename())) {
                    for (const auto& [fst, snd] : dxtypereg.get_registry()) {
                        const bool selected = tmp->get_typename() == fst;
                        // TODO: make function for this
                        ImGui::BeginDisabled(fst == "Go up" && !dxprofile.root->parent);
                        if (ImGui::Selectable(fst.c_str(), selected)) {
                            if (fst != tmp->get_typename()) {
                                shared_ptr<Item> swap = tmp;
                                if (dxprofile.root->items[dxstore.draw_item_properties]->get_typename() == fst) {
                                    tmp = dxprofile.root->items[dxstore.draw_item_properties];
                                    tmp->copy_properties(*swap);
                                } else {
                                    // TODO: Add optional copy constructor to end of signature
                                    tmp = shared_ptr<Item>(snd(nullptr, dxprofile, dxprofile.root));
                                    tmp->copy_properties(*swap);
                                }
                            }
                        }
                        if (selected)
                            ImGui::SetItemDefaultFocus();
                        ImGui::EndDisabled();
                    }
                    ImGui::EndCombo();
                }
                tmp->Item::draw_properties();
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Behavior")) {
                ImGui::BeginChild("##cfg", ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight()-ImGui::GetCursorPosY()));
                tmp->draw_properties();
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::EndChild();

        ImGui::SetCursorPos(ImVec2(0, ImGui::GetWindowHeight()-34));
        ImGui::BeginChild("##nav", ImVec2(ImGui::GetWindowWidth()-8, 26));
        ImGui::SetCursorPosX(8);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4, 0.0, 0.0, 0.54));
        if (ImGui::Button("Erase", ImVec2(64, 26))) {
            json* cfg = dxprofile.root->items[dxstore.draw_item_properties]->config;
            json& items = (*dxprofile.root->config)["items"];
            if (cfg) {
                items.erase(std::remove_if(items.begin(), items.end(), [&](const json& item) {
                    // idk maybe no index means the user made a temporary entry
                    return cfg->contains("idx") && item.contains("idx") ? item["idx"] == (*cfg)["idx"] : false;
                }), items.end());
                dxclient.update_config();
            }
            dxprofile.root->items[dxstore.draw_item_properties] = std::make_shared<ExecutableItem>(ExecutableItem(nullptr, dxprofile, dxprofile.root));
            tmp = nullptr;
            ImGui::CloseCurrentPopup();
            dxstore.draw_item_properties = -1;
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::SetCursorPosX(ImGui::GetWindowWidth()-140-68);
        if (ImGui::Button("Clear", ImVec2(64, 26))) {
            tmp = std::make_shared<ExecutableItem>(ExecutableItem(nullptr, dxprofile, dxprofile.root));
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(64, 26))) {
            tmp = dxprofile.root->items[dxstore.draw_item_properties];
            tmp->Item::properties_cancel();
            tmp->properties_cancel();
            tmp = nullptr;
            ImGui::CloseCurrentPopup();
            dxstore.draw_item_properties = -1;
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(tmp ? tmp->disabled() : true);
        if (ImGui::Button("Apply", ImVec2(64, 26))) {
            if (!tmp->get_uuid()) {
                uint64_t uuid = DX_INVALID_UUID;
                do {
                    uuid = generate_uuid();
                } while (tmp->set_uuid(uuid));
            }
            tmp->properties_init(tmp->get_typename());
            dxprofile.root->items[dxstore.draw_item_properties] = tmp;
            tmp->Item::properties_apply();
            tmp->properties_apply();
            dxprofile.parent.update_config(tmp.get());
            tmp = nullptr;
            ImGui::CloseCurrentPopup();
            dxstore.draw_item_properties = -1;
        }
        ImGui::EndDisabled();
        ImGui::EndChild();
        ImGui::EndPopup();
    }
    dxstore.lock.unlock();
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
                dxstore.lock.unlock();
                dxstore.set_selected_uuid(it.first);
                dxstore.lock.lock();
                accommodate_window_size();
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
    uint8_t rows = 4;
    uint8_t columns = 6;
    dxstore.lock.lock();
    if (selected_uuid) {
        const Client& cref = dxstore.retrieve_current_client();
        const Profile& pref = *cref.profiles[cref.current_profile];
        rows = pref.rows;
        columns = pref.columns;
    }
    ImVec2 button_size = ImVec2(std::floor(ImGui::GetWindowWidth() / (float)columns),
                                std::floor(ImGui::GetWindowHeight() / (float)rows));
    ImGui::BeginDisabled(!selected_uuid);
    for (uint8_t i = 0; i < rows*columns; ++i) {
        ImGui::PushID(i);
        bool disabled = selected_uuid ? dxstore.retrieve_current_client().get_current_profile_ref().root->items[i]->disabled() : false;
        if (disabled) {
            if (dxstore.retrieve_current_client().get_current_profile_ref().root->items[i]->label.empty())
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.0f, 0.0f, 0.54f));
        }
        ImVec2 btn_start = ImGui::GetCursorPos();
        if (selected_uuid) {
            bool pressed = false;
            shared_ptr item = dxstore.retrieve_current_client().get_current_profile_ref().root->items[i];
            ImGui::SetNextItemAllowOverlap();
            if (item->has_thumbnail && item->thumbnail->ready_to_bind())
                item->thumbnail->bind_to_context();
            if (item->has_thumbnail && item->thumbnail->is_init()) {
                pressed = ImGui::Button("##button", button_size);
                gui_overlay_texture(button_size, item->thumbnail);
            } else {
                pressed = ImGui::Button(item->label.c_str(), button_size);
            }

            if (pressed)
                dxstore.draw_item_properties = i;
            
            if (!item->disabled()) {
                if (item->openable_in_editor()) {
                    ImVec2 pos = ImGui::GetCursorPos();
                    ImGui::SetCursorPos(ImVec2(btn_start.x+button_size.x-40, btn_start.y+button_size.y-40));
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1,0.1,0.1,1));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3,0.3,0.3,1));
                    static Texture enterButton;
                    if (enterButton.is_null()) {
                        enterButton.create_from_memory(enter_icon_png, enter_icon_len);
                        while (!enterButton.ready_to_bind());
                        enterButton.bind_to_context();
                    }
                    if (ImGui::ImageButton("##->", *enterButton.get(), ImVec2(24, 24))) {
                        Profile& dxprofile = dxstore.retrieve_current_client().get_current_profile_ref();
                        item->execute();
                        extern void request_execution(uint8_t idx);
                        if (item->openable_in_editor()) {
                            request_execution(i);
                        }
                    }
                    ImGui::PopStyleColor(2);
                    ImGui::SetCursorPos(pos);
                    ImGui::SetCursorPos(btn_start);
                    ImGui::Dummy(button_size);
                }
            }
        } else {
            ImGui::Button("<empty>", button_size);
        }
        ImGui::PopID();
        if ((i+1) % columns)
            ImGui::SameLine();
        if (disabled)
            ImGui::PopStyleColor();
    }
    dxstore.lock.unlock();
    ImGui::EndDisabled();
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(4);
}

void server_main_loop() {
    ImGuiIO& io = ImGui::GetIO();
    Deckastore& dxstore = Deckastore::get();
    DxWindow& dxwindow = dxstore.window;
    status_t dxstatus = dxstore.get_status();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (dxstatus == status_t::Exiting || dxstatus == status_t::Restart) {
        gui_show_waiting_tasks();
    } else if (dxwindow.should_close()) {
        gui_close_dialog();
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

#ifdef _DEBUG
    gui_draw_performance();
#endif

    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(dxwindow.get());
}

int server_gui_init() {
    Deckastore& dxstore = Deckastore::get();
    const status_t& dxstatus = dxstore.get_status();
    // TODO: Add headless mode
    if (dxstore.create_window("Deckadence", {1080, 720})) {
        dxstore.set_status(status_t::Done);
        return -1;
    }
    DxWindow& dxwindow = dxstore.window;

    gui_init_context();

    accommodate_window_size();

    Vec2<int> size = dxwindow.get_size();

    ImGuiIO& io = ImGui::GetIO();
    while (dxstatus != status_t::Done && dxstatus != status_t::Restarting) {
        glfwWaitEventsTimeout(max_wait_time);
        if (glfwGetWindowAttrib(dxwindow.get(), GLFW_ICONIFIED)) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        if (size != dxwindow.get_size()) {
            accommodate_window_size();
            size = dxwindow.get_size();
        }

        server_main_loop();
    };
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    dxstore.destroy_window();
    glfwTerminate();
    return 0;
}
