module;

#include <unordered_map>
#include <any>

export module ZerengineCore:CompPool;

import :Utils;

class CompPool final {
friend class Archetype;
friend class LiteArchetype;
private:
    CompPool() noexcept = default;

    CompPool(const Ent ent, const std::any& a) noexcept:
        comps({{ent, a}}) {
    }

private:
    constexpr void emplace(const Ent ent, const std::any& a) noexcept {
        comps.emplace(ent, a);
    }

    [[nodiscard]] constexpr auto&& get(this auto&& self, const Ent ent) noexcept {
        return self.comps.at(ent);
    }

    constexpr void copy(const Ent ent, const CompPool& oth) noexcept {
        comps.emplace(ent, oth.comps.at(ent));
    }

    constexpr void del(const Ent ent) noexcept {
        comps.erase(ent);
    }

private:
    std::unordered_map<Ent, std::any> comps;
};