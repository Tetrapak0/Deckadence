#pragma once

#include "../../external/imgui/imgui.h"
#include "../../external/imgui/imgui_stdlib.h"
#include "../../external/imgui/imgui_impl_glfw.h"
#include "../../external/imgui/imgui_impl_opengl3.h"
#include "../../external/GLFW/include/glfw3.h"
#include "../../external/GLFW/include/glfw3native.h"

#include "../../external/OpenSans/OpenSans.h"

#include <memory>
using std::shared_ptr;

#include "Texture.hpp"

constexpr double max_wait_time = 1.0f / 60.0f;
constexpr ImGuiWindowFlags modalflags = ImGuiWindowFlags_NoMove |
                                        ImGuiWindowFlags_NoResize |
                                        ImGuiWindowFlags_NoCollapse |
                                        ImGuiWindowFlags_NoDocking;

void gui_set_colors();
void gui_set_style();
void gui_close_dialog();
void gui_show_waiting_tasks();
void gui_draw_settings();
void gui_init_context(GLFWwindowposfun move_callback = nullptr, GLFWwindowsizefun resize_callback = nullptr);
#ifdef _DEBUG
void gui_draw_performance();
#endif

void gui_overlay_texture(const ImVec2& max_size, const shared_ptr<Texture>& texture);
