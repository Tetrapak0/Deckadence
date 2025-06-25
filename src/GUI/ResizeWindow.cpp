#include "../../include/Config/Deckastore.hpp"
#include "../../include/GUI/GUI.hpp"
#include "../../include/Config/Config.hpp"

bool window_resized = false;

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
    int excess_h = (h-48) % rows;
    bool resized_too_small = false;
    if (w < columns*20) {
        resized_too_small = true;
        // printf("smaller w\n");
    }
    if (excess_w > columns / 2 || resized_too_small || w_old < w) {
        req_w = columns-excess_w;
        // printf("w bigger than columns/2\n");
    } else {
        req_w = -excess_w;
        // printf("w smaller than columns/2\n");
    }

    resized_too_small = false;
    if (h < rows*20) {
        resized_too_small = true;
        // printf("smaller h\n");
    }
    if (excess_h > rows / 2 || resized_too_small || h_old < h) {
        req_h = rows-excess_h;
        // printf("h bigger than rows/2\n");
    } else {
        req_h = -excess_h;
        // printf("h smaller than rows/2\n");
    }
    // printf("width: %d, excess: %d, required: %d\n", w, excess_w, req_w);
    // printf("height: %d, excess: %d, required: %d\n", h, excess_h, req_h);
    dxwindow.resize(Vec2<int>(w+req_w, h+req_h));
    w_old = w;
    h_old = h;
}

extern void draw_top_bar();
extern void draw_editor();
extern void draw_main_window();
extern void draw_connect_dialog();
extern void gui_draw_item_properties();
extern void gui_draw_properties();

void glfwWindowSizeCallback(GLFWwindow* window, int width, int height) {
    static Deckastore& dxstore = Deckastore::get();
    const Deckadence::mode_t dxmode = dxstore.get_mode();
    const static status_t& dxstatus = dxstore.get_status();
    static DxWindow& dxwindow = dxstore.window;
    (void)dxwindow.get_size();
    window_resized = true;
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // TODO: Unify main UI loop
    if (dxmode == Deckadence::mode_t::SERVER) {
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
    } else if (dxmode == Deckadence::mode_t::CLIENT) {
        Client& self = dxstore.retrieve_current_client();
        // TODO: Font
        if (dxstatus == status_t::EXITING) {
            gui_show_waiting_tasks();
        } else if (self.socket == NX_INVALID_SOCKET) {
            if (dxstore.draw_settings)
                gui_draw_settings();
            else
                draw_connect_dialog();
        }
        if (dxwindow.should_close()) {
            dxstore.set_status(status_t::EXITING);
            glfwSetWindowShouldClose(dxwindow.get(), GLFW_FALSE);
        }
        draw_main_window();
    }

// #ifdef _DEBUG
//     gui_draw_performance();
// #endif

    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}
