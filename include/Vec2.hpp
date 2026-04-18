#pragma once

#include <type_traits>

template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<T>>>
struct Vec2 {
    T x = 0;
    T y = 0;
    Vec2() = default;
    Vec2(T x, T y) : x(x), y(y) {}
    bool operator==(const Vec2& other) {
        return x == other.x && y == other.y;
    }
};
