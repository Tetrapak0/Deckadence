#include "../../include/Config/Config.hpp"
#include "../../include/Config/Deckastore.hpp"
#include "../../include/GUI/GUI.hpp"

#include <fstream>
#include <stdio.h>
#include <string>

using std::ifstream;
using std::ofstream;
using fs::create_directories;
using std::string;

unsigned int page = 0;
unsigned int page_bak = page;
int move = 0; // 1 = next, -1 = prev
int offset = 640;

bool cancel = false;

ImFont* heading = nullptr;
ImFont* regular = nullptr;

void draw_cancel_dialog() {
    static Deckastore& dxstore = Deckastore::get();
    ImGui::OpenPopup("Cancel Setup?");
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(280, 120));
    if (ImGui::BeginPopupModal("Cancel Setup?", nullptr, ImGuiWindowFlags_NoResize |
                                                            ImGuiWindowFlags_NoMove |
                                                            ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("Cancelling setup will terminate Deckadence.\n");
        ImGui::SetCursorPosY(ImGui::GetCursorPosY()+12);
        ImGui::Text("Cancel Setup?\n");

        ImGui::SetCursorPos(ImVec2(144, 92));
        if (ImGui::Button("Yes", ImVec2(64, 24)))
            dxstore.set_status(status_t::DONE);

        ImGui::SameLine();
        ImGui::SetCursorPosX(212);
        if (ImGui::Button("No", ImVec2(64, 24)))
            cancel = false;

        ImGui::EndPopup();
    }
}

int draw_pages() {
    static Deckastore& dxstore = Deckastore::get();
    ImGuiWindowFlags winflags = ImGuiWindowFlags_NoBackground |
                                ImGuiWindowFlags_NoDecoration |
                                ImGuiWindowFlags_NoResize     |
                                ImGuiWindowFlags_NoCollapse   |
                                ImGuiWindowFlags_NoMove       |
                                ImGuiWindowFlags_NoTitleBar;
    if (move == 1) {
        if (offset > 0) offset -= 64;
        else {
            offset = 640;
            move = 0;
            page_bak = page;
        }
    } else if (move == -1) {
        if (offset < 640) offset += 64;
        else {
            move = 0;
            page_bak = page;
        }
    }

    if (!page || ((move ==  1 && !page_bak)) ||
                  (move == -1 &&  page_bak == 1)) {
        ImGui::SetNextWindowSize(ImVec2(640, 448));
        ImGui::SetNextWindowPos(ImVec2(offset, 0), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        ImGui::Begin("#page1", nullptr, winflags);
        ImGui::PushFont(heading);
        ImGui::Text("Welcome to Deckadence");
        ImGui::PopFont();
        ImGui::Text("Press \"Next\" to begin setting up.");
        ImGui::End();
    }
    if (page == 1 || (((move ==  1 && (!page_bak || page_bak == 1)) ||
                      (move == -1 && (page_bak == 1 || page_bak == 2))))) {
        ImGui::SetNextWindowSize(ImVec2(640, 448));
        if (move == 1 && !page_bak)
            ImGui::SetNextWindowPos(ImVec2(offset+640, 0), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        else if (move == -1 && page_bak == 1)
            ImGui::SetNextWindowPos(ImVec2(offset, 0));
        else
            ImGui::SetNextWindowPos(ImVec2(offset, 0), ImGuiCond_Always, ImVec2(1.0f, 0.0f));

        ImGui::Begin("#page2", nullptr, winflags);

        ImGui::Text("Should this machine act as a server or client?\n");
        if (ImGui::RadioButton("Server", !static_cast<int>(dxstore.get_mode()))) {
            dxstore.set_mode(Deckadence::mode_t::SERVER);
        }
        ImGui::BeginDisabled(static_cast<int>(dxstore.get_mode()));
        ImGui::SetCursorPosX(ImGui::GetCursorPosX()+16);
        ImGui::Checkbox("Allow this server to be discovered by clients on your network", &dxstore.get_discoverable_ref());
        ImGui::EndDisabled();
        if (ImGui::RadioButton("Client", static_cast<int>(dxstore.get_mode()))) {
            dxstore.set_mode(Deckadence::mode_t::CLIENT);
        }

        ImGui::SetCursorPosY(428);
        ImGui::Text("*You can always change this later.");

        ImGui::End();
    }
    if (page == 2 || ((move ==  1 && (page_bak == 1 || page_bak == 2)) ||
                      (move == -1 && (page_bak == 2 || page_bak == 3)))) {
        ImGui::SetNextWindowSize(ImVec2(640, 448));
        if (move == 1 && page_bak == 1)
            ImGui::SetNextWindowPos(ImVec2(offset+640, 0), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        else if (move == -1 && page_bak == 2)
            ImGui::SetNextWindowPos(ImVec2(offset, 0));
        else
            ImGui::SetNextWindowPos(ImVec2(offset, 0), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        ImGui::Begin("#page3", NULL, winflags);
        ImGui::PushFont(heading);
        ImGui::Text("Setup finished!");
        ImGui::PopFont();
        ImGui::Text("Press \"Finish\" to start using Deckadence.");
        ImGui::End();
    }

    return 0;
}

int draw_nav() {
    static Deckastore& dxstore = Deckastore::get();
    ImGui::SetNextWindowSize(ImVec2(640, 32));
    ImGui::SetNextWindowPos(ImVec2(0, 448));
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
    ImGui::SetCursorPos(ImVec2(4, 8));
    ImGui::Text("fps: %.1f; delta %.2f; offset = %d; page, bak = %d, %d čćžđšФシ",
                io.Framerate, io.DeltaTime, offset, page, page_bak);
    ImGui::PopStyleVar();
    ImGui::SameLine();
#endif
    ImGui::SetCursorPos(ImVec2(436, 4));
    ImGui::BeginDisabled(!page);
    if (ImGui::Button("Back", ImVec2(64, 24))) {
        move = -1;
        if (page_bak != page) page_bak = page;
        --page;
        offset = 0;
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::SetCursorPos(ImVec2(504, 4));
    if (page != 2) {
        if (ImGui::Button("Next", ImVec2(64, 24))) {
            move = 1;
            if (page_bak != page) page_bak = page;
            ++page;
            offset = 640;
        }
    } else {
        if (ImGui::Button("Finish", ImVec2(64, 24)))
            dxstore.set_status(status_t::DONE);
    }
    ImGui::SameLine();
    ImGui::SetCursorPos(ImVec2(572, 4));
    if (ImGui::Button("Cancel", ImVec2(64, 24)))
        cancel = true;
    ImGui::End();
    return 0;
}

int run_oobe() {
    Deckastore& dxstore = Deckastore::get();
    const status_t& dxstatus = dxstore.get_status();
    dxstore.create_window("Set up Deckadence", {640, 480}, 0, {std::make_pair(GLFW_RESIZABLE, GLFW_FALSE)});
    const DxWindow& dxwindow = dxstore.get_window();
    if (!dxwindow.get()) {
        dxstore.set_status(status_t::DONE);
        return -1;
    }

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    ImGui::StyleColorsDark();

    gui_set_colors();

    // TODO: Unify imgui with actual app
    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 0.0f;
    style.WindowRounding = 0.0f;
    style.WindowPadding = ImVec2(4.0f, 4.0f);

    constexpr ImWchar t[] = {0x0001, 0xFFFF, 0};
    
    // NOTE: There's currently no asian language support because
    //       the glyphs use too much memory when added this way.
    regular = io.Fonts->AddFontFromMemoryCompressedTTF(OpenSans_compressed_data, OpenSans_compressed_size, 18, NULL, t);
    heading = io.Fonts->AddFontFromMemoryCompressedTTF(OpenSans_compressed_data, OpenSans_compressed_size, 36, NULL, t);

    io.Fonts->Build();

    ImGui_ImplGlfw_InitForOpenGL(dxwindow.get(), true);
    ImGui_ImplOpenGL3_Init();

    while (!static_cast<int>(dxstatus)) {
        if (move) glfwPollEvents();
        else      glfwWaitEventsTimeout(max_wait_time);
        if (glfwGetWindowAttrib(dxwindow.get(), GLFW_ICONIFIED)) {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // NOTE: Should probably make this the same as in the server
        if (dxwindow.should_close()) {
            cancel = true;
            glfwSetWindowShouldClose(dxwindow.get(), GLFW_FALSE);
        }
        if (cancel) {
            draw_cancel_dialog();
        }

        draw_pages();
        draw_nav();

        ImGui::Render();
        glViewport(0, 0, static_cast<int>(io.DisplaySize.x), static_cast<int>(io.DisplaySize.y));
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(dxwindow.get());
    };
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    dxstore.destroy_window();
    glfwTerminate();
    if (!cancel) {
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
        ofstream writer(dir / "config.json");
        writer << config.dump(4);
        writer.close();
        dxstore.set_status(status_t::RUNNING);
    }
    if (cancel) return -1;
    return 0;
}
