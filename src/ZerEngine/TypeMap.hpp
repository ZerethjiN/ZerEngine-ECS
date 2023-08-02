#pragma once

#include <unordered_map>
#include <any>

class TypeMap final {
public:
    template <typename T>
    [[nodiscard]] constexpr bool contains() const noexcept {
        return typeMap.contains(typeid(T).hash_code());
    }

    template <typename T, typename... Args>
    constexpr void emplace(Args&&... args) noexcept {
        typeMap.try_emplace(
            typeid(T).hash_code(),
            std::make_any<T>(std::forward<Args>(args)...)
        );
    }

    template <typename T>
    [[nodiscard]] constexpr T& get() noexcept {
        return std::any_cast<T&>(typeMap.at(typeid(T).hash_code()));
    }

    template <typename T>
    [[nodiscard]] constexpr const T& get() const noexcept {
        return std::any_cast<T&>(typeMap.at(typeid(T).hash_code()));
    }

    template <typename T>
    constexpr void del() noexcept {
        typeMap.erase(typeid(T).hash_code());
    }

    constexpr void clear() noexcept {
        typeMap.clear();
    }

private:
    std::unordered_map<std::size_t, std::any> typeMap;
};