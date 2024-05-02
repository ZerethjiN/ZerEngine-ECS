module;

#include <unordered_map>
#include <any>
#include <vector>
#include <unordered_set>

export module ZerengineCore:Archetype;

import :Utils;
import :CompPool;

struct ArchetypeCreateWith final {};
constinit ArchetypeCreateWith archetypeCreateWith;

struct ArchetypeCreateWithout final {};
constinit ArchetypeCreateWithout archetypeCreateWithout;

class Archetype final {
friend class Registry;
friend class LiteArchetype;
template<typename... Ts>
friend class View;
private:
    Archetype() noexcept = default;

    Archetype(const Ent ent, const std::unordered_map<Type, std::any>& anyes) noexcept:
        ents({ent}),
        pools(anyes.size()) {
        for (const auto& pair: anyes) {
            pools.emplace(pair.first, new CompPool(ent, pair.second));
        }
    }

    Archetype(ArchetypeCreateWith, Archetype& oldArch, const Ent ent, const std::any& a) noexcept:
        ents({ent}),
        pools(oldArch.size() + 1) {
        for (const auto& pair: oldArch.pools) {
            pools.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(pair.first),
                std::forward_as_tuple(new CompPool(ent, pair.second->comps.at(ent)))
            );
        }
        pools.emplace(a.type().hash_code(), new CompPool(ent, a));
        oldArch.destroy(ent);
    }

    Archetype(ArchetypeCreateWithout, Archetype& oldArch, const Ent ent, const Type type) noexcept:
        ents({ent}),
        pools(oldArch.size() - 1) {
        for (const auto& pair: oldArch.pools) {
            if (pair.first != type) {
                pools.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(pair.first),
                    std::forward_as_tuple(new CompPool(ent, pair.second->comps.at(ent)))
                );
            }
        }
        oldArch.destroy(ent);
    }

private:
    ~Archetype() noexcept {
        for (auto& pair: pools) {
            delete pair.second;
        }
    }

    void newEnt(const Ent ent, const std::unordered_map<Type, std::any>& anyes) noexcept {
        ents.emplace(ent);
        for (const auto& pair: anyes) {
            pools.at(pair.first)->emplace(ent, pair.second);
        }
    }

    void add(const Ent ent, Archetype& oldArch, const std::any& a) noexcept {
        ents.emplace(ent);
        for (const auto& pair: oldArch.pools) {
            pools.at(pair.first)->copy(ent, *pair.second);
        }
        pools.at(a.type().hash_code())->emplace(ent, a);
        oldArch.destroy(ent);
    }

    void del(const Ent ent, Archetype& oldArch, const Type type) noexcept {
        ents.emplace(ent);
        for (const auto& pair: oldArch.pools) {
            if (pair.first != type) {
                pools.at(pair.first)->copy(ent, *pair.second);
            }
        }
        oldArch.destroy(ent);
    }

    [[nodiscard]] auto&& get(this auto&& self, const Ent ent, const Type type) noexcept {
        return self.pools.at(type)->get(ent);
    }

    [[nodiscard]] const std::vector<std::string> getTypes(const Ent ent) const noexcept {
        std::vector<std::string> types;
        for (auto& pool: pools) {
            types.emplace_back(pool.second->comps.at(ent).type().name());
        }
        return types;
    }

    void destroy(const Ent ent) noexcept {
        ents.erase(ent);
        for (auto& pool: pools) {
            pool.second->del(ent);
        }
    }

    [[nodiscard]] constexpr bool empty() const noexcept {
        return ents.empty();
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept {
        return ents.size();
    }

    [[nodiscard]] constexpr bool contains(const Type type) const noexcept {
        return pools.contains(type);
    }

private:
    template <typename... Ts>
    [[nodiscard]] constexpr std::tuple<const Ent, Ts&...> getTupleWithEnt(const Ent ent) noexcept {
        return std::forward_as_tuple(ent, std::any_cast<Ts&>(get(ent, typeid(Ts).hash_code()))...);
    }

private:
    [[nodiscard]] bool isTotalyCompatibleLate(const std::unordered_map<Type, std::any>& anyes) const noexcept {
        if (anyes.size() != pools.size()) {
            return false;
        }
        for (const auto& pair: anyes) {
            if (!pools.contains(pair.first)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] bool isTotalyCompatibleLate(const Archetype& oldArch, const Type type) const noexcept {
        if (oldArch.pools.size() + 1 != pools.size()) {
            return false;
        }
        if (!pools.contains(type)) {
            return false;
        }
        for (const auto& pair: oldArch.pools) {
            if (!pools.contains(pair.first)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] bool isTotalyCompatibleWithoutLate(const Archetype& oldArch, const Type type) const noexcept {
        if (oldArch.pools.size() - 1 != pools.size() || pools.contains(type)) {
            return false;
        }
        for (const auto& pair: oldArch.pools) {
            if (!pools.contains(pair.first) && pair.first != type) {
                return false;
            }
        }
        return true;
    }

private:
    std::unordered_set<Ent> ents;
    std::unordered_map<Type, CompPool*> pools;
};