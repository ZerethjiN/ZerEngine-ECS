#pragma once

#ifdef ZERENGINE_USE_RENDER_THREAD

    #include <vector>
    #include <unordered_map>
    #include <cstring>
    #include "OSDependency.hpp"
    #include "Archetype.hpp"

    class LiteArchetype final {
    public:
        inline ~LiteArchetype() noexcept {
            free(data);
        }

        template <typename Func, typename... Ts>
        inline void each(const Func& func) const noexcept {
            if constexpr (std::is_invocable_v<Func, Ts&...>) {
                for (const auto& pair: entIdx) {
                    std::apply(func, std::forward_as_tuple(getAt<Ts>(pair.second)...));
                }
            } else if constexpr (std::is_invocable_v<Func, Ent, Ts&...>) {
                for (const auto& pair: entIdx) {
                    std::apply(func, std::forward_as_tuple(pair.first, getAt<Ts>(pair.second)...));
                }
            }
        }

        inline void copyArchetype(const Archetype& oth) noexcept {
            std::size_t chunkSize = oth.rowSize << oth.shiftMultiplier;
            data = malloc(chunkSize * oth.datas.size());
            std::size_t i = 0;
            for (auto& othData: oth.datas) {
                memcpy(
                    static_cast<std::byte*>(data) + (chunkSize * i),
                    othData,
                    chunkSize
                );
                i++;
            }
            rowSize = oth.rowSize;
            types = oth.types;
            entIdx = oth.entIdx;
        }

        [[nodiscard]] constexpr std::size_t size() const noexcept {
            return entIdx.size();
        }

        template <typename... Comps, typename... Filters, typename... Excludes>
        [[nodiscard]] constexpr bool isPartialyCompatible(const With<Filters...>& with = {}, const Without<Excludes...>& without = {}) const noexcept {
            if constexpr (sizeof...(Comps) + sizeof...(Filters) > 0) {
                if (!isCompatibleRec<Comps..., Filters...>()) {
                    return false;
                }
            }
            if constexpr (sizeof...(Excludes) > 0) {
                if (!isExcludeRec<Excludes...>()) {
                    return false;
                }
            }
            return true;
        }

    private:
        template <typename... Ts>
        [[nodiscard]] constexpr bool isCompatibleRec() const noexcept {
            return (types.contains(typeid(Ts).hash_code()) && ...);
        }

        template <typename... Ts>
        [[nodiscard]] constexpr bool isExcludeRec() const noexcept {
            return (!types.contains(typeid(Ts).hash_code()) && ...);
        }

        template <typename T>
        [[nodiscard]] constexpr T& getAt(const std::size_t idx) const noexcept {
            return reinterpret_cast<T&>(static_cast<std::byte*>(data)[rowSize * idx + types.at(typeid(T).hash_code()).offset]);
        }

    public:
        std::size_t rowSize;
        void* data;
        std::unordered_map<Type, TypeInfos> types;
        std::unordered_map<Ent, std::size_t> entIdx;
    };

#endif