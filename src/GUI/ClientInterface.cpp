#include "../../include/GUI/GUI.hpp"
#include "../../include/Config/Config.hpp"
#include "../../include/Config/Deckastore.hpp"
#include "../../include/Client/Client.hpp"
#include "../../include/Client/DiscoveryListenerService.hpp"

void request_execution(int item) {
    string command = string(1, DX_EXECUTE_BYTE) + std::to_string(item);
    send(Deckastore::get().retrieve_current_client().socket, command.c_str(), command.length()+1, 0);
}

void draw_main_window() {
    Deckastore::get().retrieve_current_client().lock.lock();
    Profile& profile = Deckastore::get().retrieve_current_client().get_current_profile_ref();
    int size = profile.rows*profile.columns;
    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags iwf = ImGuiWindowFlags_NoCollapse |
                           ImGuiWindowFlags_NoDecoration |
                           ImGuiWindowFlags_NoMove |
                           ImGuiWindowFlags_NoResize |
                           ImGuiWindowFlags_NoBringToFrontOnFocus;
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("##main", nullptr, iwf);
    for (int i = 0; i < size; ++i) {
        bool disabled = profile.items[i].command.empty();
        ImGui::BeginDisabled(disabled);
        ImGui::PushID(i);
        if (ImGui::Button(disabled ? "##" : profile.items.at(i).label.c_str(),
                          ImVec2(io.DisplaySize.x / profile.columns, io.DisplaySize.y / profile.rows))) {
            // if (item.typename == "Folder");
            // else
            printf("i: %d\n", i);
            request_execution(i);
        }
        ImGui::PopID();
        if ((i+1) % profile.columns)
            ImGui::SameLine();
        ImGui::EndDisabled();
    }
    Deckastore::get().retrieve_current_client().lock.unlock();
    ImGui::End();
    ImGui::PopStyleVar();
}

void draw_connect_dialog() {
    static bool manual_connect = false;
    static char ip_buffer[16]{0};
    static int  port = 32018;
    ImGui::OpenPopup("Connect to Server");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(420, 360));
    // TODO: Auto connect
    if (ImGui::BeginPopupModal("Connect to Server", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {
        if (manual_connect) {
            ImGui::Text("IP address: "); // TODO: Add hostname support
            ImGui::SameLine();
            ImGui::InputText("##ip", ip_buffer, 16);
            ImGui::Text("Port");
            ImGui::SameLine();
            ImGui::InputInt("##port", &port);
            if (port > 65535 || port < 0)
                port = 32018;
        } else {
            if (servers.empty()) {
                ImVec2 size = ImGui::CalcTextSize("No Deckadence servers found on your local network.");
                ImGui::SetCursorPosX((ImGui::GetWindowSize().x-size.x)*0.5f);
                ImGui::TextDisabled("No Deckadence servers found on your local network.");
            } else {
                ImVec2 size = ImGui::CalcTextSize("Deckadence servers on your local network:");
                ImGui::SetCursorPosX((ImGui::GetWindowSize().x-size.x)*0.5f);
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
        ImGui::SetCursorPosX(280);
        if (ImGui::Button("Exit", ImVec2(64, 26))) {
            glfwSetWindowShouldClose(Deckastore::get().get_window().get(), GLFW_TRUE);
        }
        ImGui::SameLine();
        ImGui::SetCursorPosX(348);
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
                    self.socket = sock;
                    res = send(self.socket, std::to_string(self.get_uuid()).c_str(), 9, 0);
                    if (!manual_connect) {
                        port = porttmp;
                        strncpy(ip_buffer, ipv4tmp, strlen(ipv4tmp));
                        ip_buffer[strlen(ipv4tmp)] = '\0';
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
    if (dxstore.create_window("Deckadence", {1280, 720}, 1)) {
        dxstore.set_status(status_t::DONE);
        return -1;
    }
    const DxWindow& dxwindow = dxstore.get_window();

    gui_init_context();

    ImGuiIO& io = ImGui::GetIO();
    //constexpr ImWchar ranges[] = {0x0001, 0xFFFF, 0};
    //ImFont* header = io.Fonts->AddFontFromMemoryCompressedTTF(OpenSans_compressed_data, OpenSans_compressed_size, 32, nullptr, ranges);
    //io.Fonts->Build();
    while (dxstatus != status_t::DONE) {
        glfwWaitEventsTimeout(max_wait_time);
        if (glfwGetWindowAttrib(dxwindow.get(), GLFW_ICONIFIED)) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        if (window_resized) {
            // accomodate_window_size(dxwindow.get());
            window_resized = false;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (dxstatus == status_t::EXITING) {
            gui_show_waiting_tasks();
        } else if (self.socket == NX_INVALID_SOCKET) {
            //ImGui::PushFont(header);
            draw_connect_dialog();
            //ImGui::PopFont();
        }
        if (dxwindow.should_close()) {
            dxstore.set_status(status_t::EXITING);
            glfwSetWindowShouldClose(dxwindow.get(), GLFW_FALSE);
        }

        draw_main_window();

#ifdef _DEBUG
        gui_draw_performance();
#endif

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
