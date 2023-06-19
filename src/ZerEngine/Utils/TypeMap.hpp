#ifndef ZERENGINE_UTILS_TYPEMAP
#define ZERENGINE_UTILS_TYPEMAP

#include <unordered_map>
#include <any>

namespace zre {
    class TypeMap {
    public:
        /**
         * @brief Check if this TypeMap Contains this Type.
         * 
         * @tparam T 
         * @return true 
         * @return false 
         */
        template <typename T>
        [[nodiscard]] constexpr bool contains() const noexcept {
            return typeMap.contains(typeid(T).hash_code());
        }

        /**
         * @brief Add a new object with its type.
         * 
         * @tparam T 
         * @param t 
         */
        template <typename T>
        constexpr void add(T&& t) noexcept {
            typeMap.try_emplace(
                typeid(T).hash_code(),
                std::make_any<T>(std::forward<T>(t))
            );
        }

        template <typename T, typename... Args>
        constexpr void emplace(Args&&... args) noexcept {
            typeMap.try_emplace(
                typeid(T).hash_code(),
                std::make_any<T>(std::forward<Args>(args)...)
            );
        }

        /**
         * @brief Get an Object by its Type.
         * 
         * @tparam T 
         * @return constexpr T& 
         */
        template <typename T>
        [[nodiscard]] constexpr T& get() noexcept {
            return std::any_cast<T&>(typeMap.at(typeid(T).hash_code()));
        }

        /**
         * @brief Get an Object by its Type.
         * 
         * @tparam T 
         * @return constexpr const T& 
         */
        template <typename T>
        [[nodiscard]] constexpr const T& get() const noexcept {
            return std::any_cast<T&>(typeMap.at(typeid(T).hash_code()));
        }

        /**
         * @brief Delete an Element by its Type.
         * 
         * @tparam T
         */
        template <typename T>
        constexpr void del() noexcept {
            typeMap.erase(typeid(T).hash_code());
        }

        /**
         * @brief Clear all elements.
         * 
         */
        inline void clear() noexcept {
            typeMap.clear();
        }

    private:
        std::unordered_map<size_t, std::any> typeMap;
    };

    namespace priv {
        using Res = TypeMap;
    }
}

#endif