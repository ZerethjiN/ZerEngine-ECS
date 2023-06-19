#pragma once

namespace zre {
    template <typename T>
    constexpr T rad(T deg) noexcept {
        return deg * static_cast<T>(0.01745329251994329576923690768489);
    }

    template <typename T>
    constexpr T deg(T rad) noexcept {
        return rad * static_cast<T>(57.295779513082320876798154814105);
    }
}