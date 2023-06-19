#pragma once

#include "LiteArchetype.hpp"

using namespace zre::priv;

namespace zre {
    template <typename... Comps>
    class LiteQuery {
    public:
        constexpr LiteQuery(const std::vector<LiteArchetype*>& newArchs) noexcept:
            archs(newArchs) {
        }

        template <typename Func>
        constexpr void each(const Func& func) const noexcept {
            for (const auto* arch: archs) {
                arch->template each<Comps...>(func);
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
        const std::vector<LiteArchetype*> archs;
    };
}