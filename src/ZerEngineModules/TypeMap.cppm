module;

#include <unordered_map>
#include <any>

export module ZerengineCore:TypeMap;

import :Utils;

class TypeMap final {
friend class World;
friend class ZerEngine;
private:
    constexpr void emplace(Type&& type, std::any&& any) noexcept {
        typeMap.emplace(std::move(type), std::move(any));
    }

    [[nodiscard]] constexpr auto&& get(this auto&& self, const Type type) noexcept {
        return self.typeMap.at(type);
    }

    constexpr void clear() noexcept {
        typeMap.clear();
    }

private:
    std::unordered_map<std::size_t, std::any> typeMap;
};