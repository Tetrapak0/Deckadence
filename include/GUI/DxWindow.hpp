#pragma once

#include <string>
#include <vector>

using std::pair;
using std::string;
using std::vector;

#include "GUI.hpp"
#include "../Vec2.hpp"

class DxWindow {
    GLFWwindow* window = nullptr;
    string title;
    int fullscreen = 0;
    Vec2<int> size;
    Vec2<int> min_size;
    Vec2<int> max_size;
    vector<pair<int, int>> hints;
public:
    [[nodiscard]] GLFWwindow* get() const;

    [[nodiscard]] bool should_close() const;

    void register_close_callback(GLFWwindowclosefun callback) const;
    void register_resize_callback(GLFWwindowsizefun callback) const;

    void resize(const Vec2<int>& size) const;
    const Vec2<int>& get_size();
    [[nodiscard]] Vec2<int> get_size_noupdate() const;

    int create(const char* title, const Vec2<int>& size = {800, 600}, int fullscreen = 0,
               const vector<pair<int, int>>& hints = {}, const Vec2<int>& min_size = {800, 600},
               const Vec2<int>& max_size = {GLFW_DONT_CARE, GLFW_DONT_CARE});
    void destroy();

    DxWindow() = default;
    ~DxWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
        hints.clear();
    }
};
