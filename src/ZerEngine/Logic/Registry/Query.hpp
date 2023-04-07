#pragma once

#include "Archetype.hpp"

using namespace zre::priv;

namespace zre {
    template <typename... Comps>
    class Query {
    public:
        constexpr Query(const std::vector<Archetype*>& newArchs) noexcept:
            archs(newArchs) {
        }

        template <typename Func>
        constexpr void each(const Func& func) const noexcept {
            for (const auto* arch: archs) {
                arch->template each<Func, Comps...>(func);
            }
        }

        [[nodiscard]] constexpr bool empty() const noexcept {
            return archs.empty();
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
    };
}