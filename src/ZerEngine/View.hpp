#pragma once

#include <vector>
#include "Archetype.hpp"
#include "Sys.hpp"

template <typename... Ts>
class View final {
public:
    constexpr View(const std::vector<const Archetype*>& newArchs, Sys& newSys) noexcept:
        archs(newArchs), sys(newSys) {
    }

    template <typename Func>
    constexpr void each(const Func& func) const noexcept {
        for (const auto* arch: archs) {
            arch->each<Func, Ts...>(func);
        }
    }

    template <typename Func>
    constexpr void parallelEach(const Func& func) const noexcept {
        if (sys.threadpool.canQueryTask()) {
            for (const auto* arch: archs) {
                sys.threadpool.addQueryTask<Func, Ts...>(func, arch);
            }
            sys.threadpool.waitQueryTask();
        } else {
            for (const auto* arch: archs) {
                arch->each<Func, Ts...>(func);
            }
        }
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        if (archs.empty()) {
            return true;
        }
        for (const auto* arch: archs) {
            if (arch->size() > 0) {
                return false;
            }
        }
        return true;
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
    Sys& sys;
};