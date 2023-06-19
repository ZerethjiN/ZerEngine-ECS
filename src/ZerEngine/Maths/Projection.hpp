#pragma once

#include "Mat3.hpp"
#include "Mat4.hpp"

namespace zre {
    template<typename T>
	[[nodiscard]] constexpr Mat4T<T> ortho(T left, T right, T bottom, T top) noexcept {
        return {
            {static_cast<float>(2) / (right - left), 0, 0, - (right + left) / (right - left)},
            {0, static_cast<float>(2) / (top - bottom), 0, - (top + bottom) / (top - bottom)},
            {0, 0, - static_cast<float>(1), 0},
            {0, 0, 0, 1}
        };
	}

    template<typename T>
    [[nodiscard]] constexpr Mat3T<T> ortho2D(T left, T right, T bottom, T top) noexcept {
        return {
            {static_cast<float>(2) / (right - left), 0, - (right + left) / (right - left)},
            {0, static_cast<float>(2) / (top - bottom), - (top + bottom) / (top - bottom)},
            {0, 0, 1}
        };
    }
}