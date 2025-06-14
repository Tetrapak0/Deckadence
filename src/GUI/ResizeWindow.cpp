#include "../../include/GUI/GUI.hpp"
#include "../../include/Config/Config.hpp"
#include "../../include/Config/Deckastore.hpp"

bool window_resized = false;

void accomodate_window_size(GLFWwindow* window) {
    static int w_old = 0, h_old = 0;
    glfwGetWindowSize(window, &w_old, &h_old);
    Deckastore& dxstore = Deckastore::get();
    static int columns = 6, rows = 4;
    if (dxstore.get_selected_uuid()) {
        const Profile& p = dxstore.retrieve_current_client().get_current_profile_ref();
        columns = p.columns;
        rows = p.rows;
    }
    int w = 0, h = 0;
    glfwGetWindowSize((GLFWwindow*)window, &w, &h);
    int req_w = 0, req_h = 0;
    int excess_w = w % columns;
    int excess_h = (h-48) % rows;
    bool resized_too_small = false;
    if (w < columns*20) {
        req_w = w - columns*20;
        resized_too_small = true;
        printf("smaller w\n");
    }
    if (excess_w > columns / 2 || resized_too_small || w_old < w) {
        req_w = columns-excess_w;
        printf("w bigger than columns/2\n");
    } else {
        req_w = -excess_w;
        printf("w smaller than columns/2\n");
    }

    resized_too_small = false;
    if (h < rows*20) {
        req_h = h - rows*20;
        resized_too_small = true;
        printf("smaller h\n");
    }
    if (excess_h > rows / 2 || resized_too_small || h_old < h) {
        req_h = rows-excess_h;
        printf("h bigger than rows/2\n");
    } else {
        req_h = -excess_h;
        printf("h smaller than rows/2\n");
    }
    printf("width: %d, excess: %d, required: %d\n", w, excess_w, req_w);
    printf("height: %d, excess: %d, required: %d\n", h, excess_h, req_h);
    glfwSetWindowSize((GLFWwindow*)window, w+req_w, h+req_h);
    w_old = w;
    h_old = h;
}

extern void draw_top_bar();
extern void draw_editor();
extern void draw_main_window();
extern void gui_close_dialog();
extern void draw_connect_dialog();
extern void gui_draw_item_properties();
extern void gui_draw_settings();
extern void gui_draw_properties();

void glfwWindowSizeCallback(GLFWwindow* window, int width, int height) {
    static Deckastore& dxstore = Deckastore::get();
    const Deckadence::mode_t dxmode = dxstore.get_mode();
    const static status_t& dxstatus = dxstore.get_status();
    const static DxWindow& dxwindow = dxstore.get_window();
    window_resized = true;
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // TODO: Unify main UI loop
    if (dxmode == Deckadence::mode_t::SERVER) {
        if (dxstore.draw_properties)
            dxstore.retrieve_current_client().draw_properties();
        if (dxstore.draw_settings)
            gui_draw_settings();
        if (dxstore.draw_item_properties != -1)
            dxstore.retrieve_current_client().get_current_profile_ref().items[dxstore.draw_item_properties].draw_properties();
        if (dxwindow.should_close())
            gui_close_dialog(dxwindow.get());
        if (dxstatus == status_t::EXITING || dxstatus == status_t::RESTART)
            gui_show_waiting_tasks();
        draw_top_bar();
        draw_editor();
    } else if (dxmode == Deckadence::mode_t::CLIENT) {
        Client& self = dxstore.retrieve_current_client();
        if (dxstatus == status_t::EXITING) {
            gui_show_waiting_tasks();
        } else if (self.socket == NX_INVALID_SOCKET) {
            //ImGui::PushFont(header); // TODO: Font
            draw_connect_dialog();
            //ImGui::PopFont();
        }
        if (dxwindow.should_close()) {
            dxstore.set_status(status_t::EXITING);
            glfwSetWindowShouldClose(dxwindow.get(), GLFW_FALSE);
        }
        draw_main_window();
    }

#ifdef _DEBUG
    gui_draw_performance();
#endif

    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
}
