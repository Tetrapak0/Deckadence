#include "Config/Deckastore.hpp"
#include "Config/Config.hpp"
#include "GUI/GUI.hpp"
#include "GUI/Texture.hpp"
extern "C" {
    #include "../../resources/icon.h"
}

#include <fstream>
#include <stdio.h>
#include <string>

#include "../../external/stb/stb_image.h"

using std::ifstream;
using std::ofstream;
using fs::create_directories;
using std::string;

unsigned int page = 0;
unsigned int page_bak = page;
int pageMoveInProgress = 0; // 1 = next, -1 = prev
int offset = 640;

int port = 32018;

bool cancel = false;

Texture icon;

void draw_cancel_dialog() {
    if (!cancel)
        cancel = true;
    static Deckastore& dxstore = Deckastore::get();
    if (!ImGui::IsPopupOpen("Cancel Setup?"))
        ImGui::OpenPopup("Cancel Setup?");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(280, 160));
    if (ImGui::BeginPopupModal("Cancel Setup?", nullptr, ImGuiWindowFlags_NoResize |
                                                            ImGuiWindowFlags_NoMove |
                                                            ImGuiWindowFlags_NoCollapse)) {
        ImGui::TextWrapped("Cancelling setup will terminate Deckadence.\n");
        ImGui::SetCursorPosY(ImGui::GetCursorPosY()+12);
        ImGui::Text("Cancel Setup?\n");
        ImGui::SetCursorPos(ImVec2(140, 126));
        if (ImGui::Button("Yes", ImVec2(64, 26))) {
            dxstore.set_status(status_t::Done);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("No", ImVec2(64, 26))) {
            glfwSetWindowShouldClose(dxstore.window.get(), GLFW_FALSE);
            cancel = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void draw_pages() {
    static Deckastore& dxstore = Deckastore::get();
    ImGuiWindowFlags winflags = ImGuiWindowFlags_NoBackground |
                                ImGuiWindowFlags_NoDecoration |
                                ImGuiWindowFlags_NoResize     |
                                ImGuiWindowFlags_NoCollapse   |
                                ImGuiWindowFlags_NoMove       |
                                ImGuiWindowFlags_NoTitleBar;
    if (pageMoveInProgress == 1) {
        if (offset > 0) offset -= 64;
        else {
            offset = 640;
            pageMoveInProgress = 0;
            page_bak = page;
        }
    } else if (pageMoveInProgress == -1) {
        if (offset < 640) offset += 64;
        else {
            pageMoveInProgress = 0;
            page_bak = page;
        }
    }

    if (!page || (pageMoveInProgress ==  1 && !page_bak) ||
                 (pageMoveInProgress == -1 &&  page_bak == 1)) {
        ImGui::SetNextWindowSize(ImVec2(640, 438));
        ImGui::SetNextWindowPos(ImVec2(offset, 0), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        ImGui::Begin("#page1", nullptr, winflags);
        ImGui::PushFont(dxstore.header);
        ImGui::Text("Welcome to Deckadence");
        ImGui::PopFont();
        ImGui::Text("Press \"Next\" to begin setting up.");
        if (icon.ready_to_bind()) {
            icon.bind_to_context();
        } else if (icon.is_init()) {
            ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth()-136, ImGui::GetWindowHeight()-136));
            ImGui::Image(*icon.get(), ImVec2(128, 128));
        }
        ImGui::End();
    }
    if (page == 1 || (pageMoveInProgress ==  1 && (!page_bak || page_bak == 1)) ||
                     (pageMoveInProgress == -1 && (page_bak == 1 || page_bak == 2))) {
        ImGui::SetNextWindowSize(ImVec2(640, 438));
        if (pageMoveInProgress == 1 && !page_bak)
            ImGui::SetNextWindowPos(ImVec2(offset+640, 0), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        else if (pageMoveInProgress == -1 && page_bak == 1)
            ImGui::SetNextWindowPos(ImVec2(offset, 0));
        else
            ImGui::SetNextWindowPos(ImVec2(offset, 0), ImGuiCond_Always, ImVec2(1.0f, 0.0f));

        ImGui::Begin("#page2", nullptr, winflags);
        ImGui::Text("Should this machine act as a server or client?\n");
        if (ImGui::RadioButton("Server", !static_cast<int>(dxstore.get_mode()))) {
            dxstore.set_mode(Deckadence::mode_t::Server);
        }
        ImGui::SetCursorPosX(24);
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Server port: ");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(256);
        static constexpr unsigned short step = 1;
        static constexpr unsigned short step_fast = 100;
        ImGui::InputScalar("##port", ImGuiDataType_U16, &port, &step, &step_fast);
        ImGui::BeginDisabled(static_cast<int>(dxstore.get_mode()));
        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+16);
        ImGui::Checkbox("Allow this server to be discovered by clients on your network", &dxstore.get_discoverable_ref());
        ImGui::EndDisabled();
        if (ImGui::RadioButton("Client", static_cast<int>(dxstore.get_mode()))) {
            dxstore.set_mode(Deckadence::mode_t::Client);
        }
        ImGui::SetCursorPosY(418);
        ImGui::Text("*You can always change this later.");
        ImGui::End();
    }
    if (page == 2 || ((pageMoveInProgress ==  1 && (page_bak == 1 || page_bak == 2)) ||
                      (pageMoveInProgress == -1 && (page_bak == 2 || page_bak == 3)))) {
        ImGui::SetNextWindowSize(ImVec2(640, 438));
        if (pageMoveInProgress == 1 && page_bak == 1)
            ImGui::SetNextWindowPos(ImVec2(offset+640, 0), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        else if (pageMoveInProgress == -1 && page_bak == 2)
            ImGui::SetNextWindowPos(ImVec2(offset, 0));
        else
            ImGui::SetNextWindowPos(ImVec2(offset, 0), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        ImGui::Begin("#page3", nullptr, winflags);
        ImGui::PushFont(dxstore.header);
        ImGui::Text("Setup finished!");
        ImGui::PopFont();
        ImGui::Text("Press \"Finish\" to start using Deckadence.");
        if (icon.ready_to_bind()) {
            icon.bind_to_context();
        } else if (icon.is_init()) {
            ImGui::SetCursorPos(ImVec2(ImGui::GetWindowWidth()-136, ImGui::GetWindowHeight()-136));
            ImGui::Image(*icon.get(), ImVec2(128, 128));
        }
        ImGui::End();
    }
}

void draw_nav() {
    static Deckastore& dxstore = Deckastore::get();
    ImGui::SetNextWindowPos(ImVec2(0, 438));
    ImGui::SetNextWindowSize(ImVec2(640, 42));
    ImGuiWindowFlags winflags = ImGuiWindowFlags_NoBackground |
                                ImGuiWindowFlags_NoDecoration |
                                ImGuiWindowFlags_NoResize     |
                                ImGuiWindowFlags_NoCollapse   |
                                ImGuiWindowFlags_NoMove       |
                                ImGuiWindowFlags_NoTitleBar;
    ImGui::Begin("#navbar", nullptr, winflags);
#ifdef _DEBUG
    static ImGuiIO& io = ImGui::GetIO();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::Text("fps: %.1f; delta %.2f; offset = %d; page, bak = %d, %d čćžđšФシ",
                io.Framerate, io.DeltaTime, offset, page, page_bak);
    ImGui::PopStyleVar();
    ImGui::SameLine();
#endif
    ImGui::SetCursorPosX(432);
    ImGui::BeginDisabled(!page);
    if (ImGui::Button("Back", ImVec2(64, 26))) {
        pageMoveInProgress = -1;
        if (page_bak != page) page_bak = page;
        --page;
        offset = 0;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (page != 2) {
        if (ImGui::Button("Next", ImVec2(64, 26))) {
            pageMoveInProgress = 1;
            if (page_bak != page) page_bak = page;
            ++page;
            offset = 640;
        }
    } else {
        if (ImGui::Button("Finish", ImVec2(64, 26)))
            dxstore.set_status(status_t::Done);
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(64, 26))) {
        glfwSetWindowShouldClose(dxstore.window.get(), GLFW_TRUE);
    }
    ImGui::End();
}

void oobe_main_loop() {
    ImGuiIO& io = ImGui::GetIO();
    const DxWindow& dxwindow = Deckastore::get().window;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (dxwindow.should_close())
        draw_cancel_dialog();

    draw_pages();
    draw_nav();

    ImGui::Render();
    glViewport(0, 0, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(dxwindow.get());
}

int run_oobe() {
    Deckastore& dxstore = Deckastore::get();
    port = dxstore.get_port();
    const status_t& dxstatus = dxstore.get_status();
    dxstore.create_window("Set up Deckadence", {640, 480}, 0, {std::make_pair(GLFW_RESIZABLE, GLFW_FALSE)});
    const DxWindow& dxwindow = dxstore.window;
    if (!dxwindow.get()) {
        dxstore.set_status(status_t::Done);
        return -1;
    }

    dxstore.add_task(tasks::Setup);

    gui_init_context();

    ImGuiIO& io = ImGui::GetIO();
    icon.create_from_memory(icon_png, icon_png_len);
    while (!static_cast<int>(dxstatus)) {
        if (pageMoveInProgress)
            glfwPollEvents();
        else
            glfwWaitEventsTimeout(max_wait_time);
        if (glfwGetWindowAttrib(dxwindow.get(), GLFW_ICONIFIED)) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        oobe_main_loop();
    };
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    dxstore.destroy_window();
    icon.destroy();
    glfwTerminate();
    if (cancel) {
        dxstore.remove_task(tasks::Setup);
        return -1;
    }
    fs::path dir = get_cfg_dir();
    create_directories(dir);
    json config;
    if (fs::exists(dir / "config.json")) {
        ifstream reader(dir/"config.json");
        config = json::parse(reader);
        reader.close();
    }
    config["mode"] = static_cast<int>(dxstore.get_mode());
    config["server"]["discoverable"] = dxstore.get_discoverable();
    config["server"]["port"] = port;
    ofstream writer(dir / "config.json");
    writer << config.dump(4);
    writer.close();
    page = 0;
    page_bak = 0;
    dxstore.set_status(status_t::Running);
    dxstore.remove_task(tasks::Setup);
    return 0;
}
