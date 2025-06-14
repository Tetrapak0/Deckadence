#pragma once

#include "../../external/imgui/imgui.h"
#include "../../external/imgui/imgui_stdlib.h"
#include "../../external/imgui/imgui_impl_glfw.h"
#include "../../external/imgui/imgui_impl_opengl3.h"
#include "../../external/GLFW/include/glfw3.h"
#include "../../external/GLFW/include/glfw3native.h"

#include "../../external/OpenSans/OpenSans.h"

constexpr double max_wait_time = 1.0f / 60.0f;
constexpr ImGuiWindowFlags modalflags = ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoCollapse |
                                        ImGuiWindowFlags_NoDocking;

#ifdef _DEBUG
extern void gui_draw_performance();
#endif

extern void gui_set_colors();
extern void gui_set_style();
extern void gui_close_dialog(GLFWwindow* window);
extern void gui_show_waiting_tasks();
extern void gui_init_context();

extern void accomodate_window_size(GLFWwindow* window);

extern void glfwWindowSizeCallback(GLFWwindow* window, int width, int height);

extern bool window_resized;
