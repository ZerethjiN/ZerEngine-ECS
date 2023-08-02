#pragma once

#ifdef ZERENGINE_USE_RENDER_THREAD

    #include <vector>
    #include "LiteArchetype.hpp"

    template <typename... Ts>
    class LiteView final {
    public:
        constexpr LiteView(const std::vector<const LiteArchetype*>& newArchs) noexcept:
            archs(newArchs) {
        }

        template <typename Func>
        constexpr void each(const Func& func) const noexcept {
            for (const auto* arch: archs) {
                arch->each<Func, Ts...>(func);
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
        const std::vector<const LiteArchetype*> archs;
    };

#endif