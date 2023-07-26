#pragma once

#ifdef ZERENGINE_USE_RENDER_THREAD

    #include "View.hpp"
    #include "Archetype.hpp"
    #include "LiteArchetype.hpp"
    #include "LiteView.hpp"

    class LiteRegistry final {
    public:
        inline ~LiteRegistry() noexcept {
            for (auto* arch: archs) {
                delete arch;
            }
        }

        template <typename... Comps, typename... Filters, typename... Excludes>
        [[nodiscard]] inline const LiteView<Comps...> query(const With<Filters...>& with = {}, const Without<Excludes...>& without = {}) noexcept {
            std::vector<LiteArchetype*> viewArchs;
            const constexpr std::size_t minlength = sizeof...(Comps) + sizeof...(Filters) - sizeof...(Excludes);
            for (auto* arch: archs) {
                if (arch->types.size() >= minlength) {
                    if (arch->isPartialyCompatible<Comps...>(with, without)) {
                        viewArchs.push_back(arch);
                    }
                }
            }
            return {viewArchs};
        }

        template <typename... Comps>
        constexpr void copyFromView(const View<Comps...>& query) noexcept {
            for (const auto* arch: query.archs) {
                archs.emplace_back(new LiteArchetype())->copyArchetype(*arch);
            }
        }

        constexpr void clear() noexcept {
            for (auto* arch: archs) {
                delete arch;
            }
            archs.clear();
        }

    private:
        std::vector<LiteArchetype*> archs;
    };

#endif