#include "Config/Deckastore.hpp"
#include "Client/Client.hpp"
#include "GUI/GUI.hpp"
#include "Config/Config.hpp"
#include "Client/DiscoveryListenerService.hpp"
#include "Server/Message.hpp"

void request_execution(const uint8_t idx) {
    uint64_t header = construct_header(1, static_cast<uint32_t>(MessageType::Execute));
    string msg;
    msg.append(reinterpret_cast<const char*>(&header), sizeof(uint64_t));
    msg.append(reinterpret_cast<const char*>(&idx), 1);
    send(Deckastore::get().retrieve_current_client().socket, msg.c_str(), msg.length(), 0);
}

void draw_main_window() {
    Deckastore& dxstore = Deckastore::get();
    dxstore.retrieve_current_client().lock.lock(); // Why? Why not main lock?
    Profile& dxprofile = dxstore.retrieve_current_client().get_current_profile_ref();
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags iwf = ImGuiWindowFlags_NoCollapse |
                           ImGuiWindowFlags_NoDecoration |
                           ImGuiWindowFlags_NoMove |
                           ImGuiWindowFlags_NoResize |
                           ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyle().Colors[ImGuiCol_Button]);
    ImGui::Begin("##main", nullptr, iwf);
    ImVec2 button_size = ImVec2(std::floor(io.DisplaySize.x / (float)dxprofile.columns),
                                std::floor(io.DisplaySize.y / (float)dxprofile.rows));
    for (uint8_t i = 0; i < dxprofile.rows*dxprofile.columns; ++i) {
        bool disabled = dxprofile.root->items[i]->disabled();
        if (disabled)
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::BeginDisabled(disabled);
        ImGui::PushID(i);
            bool pressed = false;
            shared_ptr item = dxprofile.root->items[i];
            if (item->has_thumbnail && item->thumbnail->ready_to_bind())
                item->thumbnail->bind_to_context();
            if (item->has_thumbnail && item->thumbnail->is_init()) {
                pressed = ImGui::Button("##button", button_size);
                gui_overlay_texture(button_size, item->thumbnail);
            } else {
                pressed = ImGui::Button(item->label.c_str(), button_size);
            }
        if (pressed) {
            request_execution(i);
            if (item->openable_in_editor()) {
                item->execute();
            }
        }

        ImGui::PopID();
        if ((i+1) % dxprofile.columns)
            ImGui::SameLine();
        ImGui::EndDisabled();
        if (disabled) {
            ImGui::PopStyleColor();
        }
    }
    dxstore.retrieve_current_client().lock.unlock();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(3);
    ImGui::End();
}

void draw_connect_dialog() {
    static bool manual_connect = false;
    static char ip_buffer[16]{};
    static int  port = 32018;
    if (!ImGui::IsPopupOpen("Connect to Server"))
        ImGui::OpenPopup("Connect to Server");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(420, 360));
    // TODO: Auto connect
    if (ImGui::BeginPopupModal("Connect to Server", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {
        if (manual_connect) {
            // TODO: Add hostname support
            ImGui::Text("IP address: ");
            ImGui::SameLine();
            ImGui::InputText("##ip", ip_buffer, 16);
            ImGui::Text("Port");
            ImGui::SameLine();
            // TODO: Make a global collection of these
            static constexpr unsigned short step = 1;
            static constexpr unsigned short step_fast = 100;
            ImGui::InputScalar("##port", ImGuiDataType_U16, &port, &step, &step_fast);
        } else {
            if (selected_server >= servers.size()) {
                selected_server = -1;
            }
            if (servers.empty()) {
                ImVec2 size = ImGui::CalcTextSize("No Deckadence servers found on your local network.");
                ImGui::SetCursorPosX((ImGui::GetWindowWidth()-size.x)*0.5f);
                ImGui::TextDisabled("No Deckadence servers found on your local network.");
            } else {
                ImVec2 size = ImGui::CalcTextSize("Deckadence servers on your local network:");
                ImGui::SetCursorPosX((ImGui::GetWindowWidth()-size.x)*0.5f);
                ImGui::TextDisabled("Deckadence servers on your local network:");
                for (int i = 0; i < servers.size(); ++i) {
                    const bool selected = selected_server == i;
                    servers_lock.lock();
                    if (ImGui::Selectable(servers[i].full.c_str(), selected, ImGuiSelectableFlags_DontClosePopups)) {
                        selected_server = i;
                    }
                    servers_lock.unlock();
                }
            }
        }
        ImGui::SetCursorPosY(326);
        ImGui::Checkbox("Enter manually", &manual_connect);
        ImGui::SameLine();
        ImGui::SetCursorPosX(216);
        if (ImGui::Button("Settings", ImVec2(64, 26))) {
            ImGui::CloseCurrentPopup();
            Deckastore::get().draw_settings = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Exit", ImVec2(64, 26))) {
            ImGui::CloseCurrentPopup();
            glfwSetWindowShouldClose(Deckastore::get().window.get(), GLFW_TRUE);
        }
        ImGui::SameLine();
        ImGui::BeginDisabled((!manual_connect && selected_server == -1) || (manual_connect && !ip_buffer[0]));
        if (ImGui::Button("Connect")) {
            servers_lock.lock();
            uint16_t porttmp = 0;
            const char* ipv4tmp = nullptr;
            if (manual_connect) {
                ipv4tmp = ip_buffer;
                porttmp = port;
            } else {
                ipv4tmp = servers[selected_server].ipv4.c_str();
                porttmp = servers[selected_server].port;
            }
            Client& self = Deckastore::get().retrieve_current_client();
            socket_t sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (sock != NX_SOCKET_ERROR) {
                sockaddr_in hint{};
                hint.sin_family = AF_INET;
                hint.sin_port = htons(porttmp);
                inet_pton(AF_INET, ipv4tmp, &hint.sin_addr);
                int res = connect(sock, (sockaddr*)&hint, sizeof(hint));
                if (res != NX_SOCKET_ERROR) {
                    res = send(sock, std::to_string(self.get_uuid()).c_str(), std::to_string(self.get_uuid()).length()+1, 0);
                    if (res > 0) {
                        self.socket = sock;
                        if (!manual_connect) {
                            port = porttmp;
                            strncpy(ip_buffer, ipv4tmp, strlen(ipv4tmp));
                            ip_buffer[strlen(ipv4tmp)] = '\0';
                        }
                        selected_server = -1;
                        ImGui::CloseCurrentPopup();
                    } else {
                        nx_sock_close(sock);
                    }
                } else {
                    nx_sock_close(sock);
                }
            }
            servers_lock.unlock();
        }
        ImGui::EndDisabled();
        ImGui::EndPopup();
    }
}

int client_gui_init() {
    Deckastore& dxstore = Deckastore::get();
    const status_t& dxstatus = dxstore.get_status();
    Client& self = dxstore.retrieve_current_client();
#ifdef _DEBUG
    if (dxstore.create_window("Deckadence", {1280, 720}, -1)) {
#else
    if (dxstore.create_window("Deckadence", {1280, 720}, 1)) {
#endif
        dxstore.set_status(status_t::Done);
        return -1;
    }
    const DxWindow& dxwindow = dxstore.window;

    gui_init_context();

    ImGuiIO& io = ImGui::GetIO();
    //constexpr ImWchar ranges[] = {0x0001, 0xFFFF, 0};
    //ImFont* header = io.Fonts->AddFontFromMemoryCompressedTTF(OpenSans_compressed_data, OpenSans_compressed_size, 32, nullptr, ranges);
    //io.Fonts->Build();
    while (dxstatus != status_t::Done && dxstatus != status_t::Restarting) {
        glfwWaitEventsTimeout(max_wait_time);
        if (glfwGetWindowAttrib(dxwindow.get(), GLFW_ICONIFIED)) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // TODO: Font
        //ImGui::PushFont(header);
        if (dxstatus == status_t::Exiting || dxstatus == status_t::Restart) {
            gui_show_waiting_tasks();
        } else if (self.socket == NX_INVALID_SOCKET) {
            if (glfwGetInputMode(dxstore.window.get(), GLFW_CURSOR) != GLFW_CURSOR_NORMAL)
                glfwSetInputMode(dxstore.window.get(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            if (dxstore.draw_settings)
                gui_draw_settings();
            else
                draw_connect_dialog();
        } else {
            if (glfwGetInputMode(dxstore.window.get(), GLFW_CURSOR) != GLFW_CURSOR_HIDDEN)
                glfwSetInputMode(dxstore.window.get(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        }
        if (dxwindow.should_close()) {
            dxstore.set_status(status_t::Exiting);
            glfwSetWindowShouldClose(dxwindow.get(), GLFW_FALSE);
        }
        draw_main_window();
        //ImGui::PopFont();
// #ifdef _DEBUG
//         gui_draw_performance();
// #endif

        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(dxwindow.get());
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    dxstore.destroy_window();
    glfwTerminate();
    return 0;
}
