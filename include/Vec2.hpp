#pragma once

#include <type_traits>

template <typename T, typename = std::enable_if_t<std::is_same_v<T, int8_t> ||
                                                  std::is_same_v<T, uint8_t> ||
                                                  std::is_same_v<T, int16_t> ||
                                                  std::is_same_v<T, uint16_t> ||
                                                  std::is_same_v<T, int32_t> ||
                                                  std::is_same_v<T, uint32_t> ||
                                                  std::is_same_v<T, int64_t> ||
                                                  std::is_same_v<T, uint64_t> ||
                                                  std::is_same_v<T, float> ||
                                                  std::is_same_v<T, double>>>
struct Vec2 {
    T x = 0;
    T y = 0;
    Vec2() = default;
    Vec2(T x, T y) : x(x), y(y) {}
};
