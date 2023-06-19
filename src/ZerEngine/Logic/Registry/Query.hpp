#pragma once

#include "Archetype.hpp"
#include "../Systems/Systems.hpp"

using namespace zre::priv;

namespace zre {
    template <typename... Comps>
    class Query {
    public:
        constexpr Query(const std::vector<Archetype*>& newArchs, Sys& newSys) noexcept:
            archs(newArchs), sys(newSys) {
        }

        template <typename Func>
        constexpr void each(const Func& func) const noexcept {
            for (const auto* arch: archs) {
                arch->template each<Func, Comps...>(func);
            }
        }

        template <typename Func>
        constexpr void parallelEach(const Func& func) const noexcept {
            if (sys.threadpool.canQueryTask()) {
                for (const auto* arch: archs) {
                    sys.threadpool.addQueryTask<Func, Comps...>(func, arch);
                }
                sys.threadpool.waitQueryTask();
            } else {
                for (const auto* arch: archs) {
                    arch->template each<Func, Comps...>(func);
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

        [[nodiscard]] constexpr size_t size() const noexcept {
            size_t length = 0;
            for (const auto* arch: archs) {
                length += arch->size();
            }
            return length;
        }

    public:
        const std::vector<Archetype*> archs;
        Sys& sys;
    };
}