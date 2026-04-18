#include "Config/Deckastore.hpp"
#include "Config/Config.hpp"
#include "Client/Client.hpp"
#include "Server/Server.hpp"
#include "Utilities/SingleInstanceSocket.hpp"

#include <thread>

// TODO: Keep checking if config file has been changed and reload if it has been
// TODO: Installer

Deckadence::mode_t mode_selector(bool& multi_instance) {
    Deckastore& dxstore = Deckastore::get();
    dxstore.create_window("Launch Deckadence", {256, 200}, 0, {{GLFW_RESIZABLE, GLFW_FALSE}});
    DxWindow& dxwindow = dxstore.window;

    gui_init_context();

    ImGuiIO& io = ImGui::GetIO();

    int mode = 0;

    while (true) {
        glfwWaitEventsTimeout(max_wait_time);
        if (glfwGetWindowAttrib(dxwindow.get(), GLFW_ICONIFIED)) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        if (dxwindow.should_close()) {
            mode = static_cast<int>(Deckadence::mode_t::Undefined);
            break;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(256, 200));

        ImGui::Begin("##", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        ImGui::Text("Select launch mode:");
        if (ImGui::RadioButton("Server", !mode)) {
            mode = 0;
        }
        if (ImGui::RadioButton("Client", mode == 1)) {
            mode = 1;
        }
        ImGui::Text("\n");
        ImGui::Checkbox("Allow multiple instances", &multi_instance);

        ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth() - 136, ImGui::GetWindowHeight() - 30));
        if (ImGui::Button("Ok", ImVec2(64,26))) {
            ImGui::End();
            break;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(64,26))) {
            mode = static_cast<int>(Deckadence::mode_t::Undefined);
            ImGui::End();
            break;
        }

        ImGui::End();

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

    return static_cast<Deckadence::mode_t>(mode);
}

int main(int argc, char** argv) {
    setbuf(stdout, nullptr);
    setbuf(stderr, nullptr);
    int ret = 0;
    Deckastore& dxstore = Deckastore::get();
    if (nx_sock_init()) {
    #ifdef _WIN32
        printf("Failed to initialize networking. WSAGetLastError(): %d\n", WSAGetLastError());
    #else
        printf("Failed to initialize networking: %s\n", strerror(errno));
    #endif
        return -1;
    }

    dxstore.set_status(status_t::Restart);
#ifdef _DEBUG
    bool multi_instance = false;
    Deckadence::mode_t mode = Deckadence::mode_t::Server;
    bool override_mode = false;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-M"))
            multi_instance = true;
        if (!strcmp(argv[i], "-S") && !override_mode) {
            mode = Deckadence::mode_t::Server;
            override_mode = true;
        } else if (!strcmp(argv[i], "-C") && !override_mode) {
            mode = Deckadence::mode_t::Client;
            override_mode = true;
        }
    }
    if (!override_mode) {
        mode = mode_selector(multi_instance);
        override_mode = true;
    }
    if (multi_instance || mode == Deckadence::mode_t::Undefined) {
        Deckastore::get().disable_sis();
        dxstore.set_status(status_t::Running);
    }
#endif
    std::thread t_si_ensurer(si_socket_init);

    if (mode == Deckadence::mode_t::Undefined)
        dxstore.set_status(status_t::Done);

    while (dxstore.get_status() == status_t::Restart) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    while (dxstore.status != status_t::Done) {
        Deckastore::reset();
        if (check_config()) {
            ret = -1;
            break;
        }
#ifdef _DEBUG
        if (override_mode)
            Deckastore::get().set_mode(mode);
#endif
        // TODO: Should probably implement return value
        dxstore.net_query_interfaces();
        switch (static_cast<int>(dxstore.mode)) {
            case static_cast<int>(Deckadence::mode_t::Server): {
                ret = start_server_sequence();
                break;
            } case static_cast<int>(Deckadence::mode_t::Client): {
                ret = start_client_sequence();
                break;
            } default: {
                ret = -1;
                break;
            }
        }
    }
    t_si_ensurer.join();
    nx_sock_cleanup();
    return ret;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pCmdLine, int nCmdShow) {
#ifdef _DEBUG
    AllocConsole();
    AttachConsole(GetCurrentProcessId());
#ifdef _MSC_VER
    freopen("CON", "w", stdout);
    freopen("CON", "w", stderr);
    freopen("CON", "r", stdin);
#else
    freopen("CONIN$",  "w", stdout);
    freopen("CONOUT$", "w", stderr);
    freopen("CONOUT$", "r", stdin);
#endif
#endif
    int ret = main(__argc, __argv);
#ifdef _DEBUG
    FreeConsole();
#endif
    return ret;
}
#endif
