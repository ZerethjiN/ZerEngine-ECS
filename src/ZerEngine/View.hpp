#pragma once

#include <vector>
#include "Archetype.hpp"

template <typename... Ts>
class View final {
public:
    constexpr View(const std::vector<const Archetype*>& newArchs) noexcept:
        archs(newArchs) {
    }

    template <typename Func>
    constexpr void each(const Func& func) const noexcept {
        for (const auto* arch: archs) {
            arch->each<Func, Ts...>(func);
        }
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept {
        std::size_t newSize = 0;
        for (const auto* arch: archs) {
            newSize += arch->size();
        }
        return newSize;
    }

private:
    const std::vector<const Archetype*> archs;
};