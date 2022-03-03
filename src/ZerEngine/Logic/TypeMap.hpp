/**
 * @file TypeMap.hpp
 * @author ZerethjiN
 * @brief Like a map, but with type in entry and
 * associated objects with the same type.
 * @version 0.1
 * @date 2022-02-22
 * 
 * @copyright Copyright (c) 2022 - ZerethjiN
 * 
 */
#ifndef ZERENGINE_RESSOURCE_HPP
#define ZERENGINE_RESSOURCE_HPP

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
            return typeMap.count(typeid(T).hash_code());
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

    private:
        std::unordered_map<size_t, std::any> typeMap;
    };
}

#endif // ZERENGINE_RESSOURCE_HPP