#pragma once

#include <any>
#include <cmath>
#include <chrono>
#include <concepts>
#include <condition_variable>
#include <cstddef>
#include <cstdlib>
#include <ctime>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <mutex>
#include <optional>
#include <print>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using Ent = std::size_t;
using Type = std::size_t;

template <typename... Filters>
class With final {};
template <typename... Filters>
static constexpr inline With<Filters...> with;

template <typename... Excludes>
class Without final {};
template <typename... Excludes>
static constexpr inline Without<Excludes...> without;

class WithInactive final {};
static constexpr inline WithInactive withInactive;

class IsInactive final {};
class DontDestroyOnLoad final {};

class ISystem {
protected:
    constexpr ISystem() noexcept = default;
};

class StartSystem final: protected ISystem {};
static constexpr inline StartSystem startSystem;
class MainSystem final: protected ISystem {};
static constexpr inline MainSystem mainSystem;
class MainFixedSystem final: protected ISystem {};
static constexpr inline MainFixedSystem mainFixedSystem;
class MainUnscaledFixedSystem final: protected ISystem {};
static constexpr inline MainUnscaledFixedSystem mainUnscaledFixedSystem;
class ThreadedSystem final: protected ISystem {};
static constexpr inline ThreadedSystem threadedSystem;
class ThreadedFixedSystem final: protected ISystem {};
static constexpr inline ThreadedFixedSystem threadedFixedSystem;
class ThreadedUnscaledFixedSystem final: protected ISystem {};
static constexpr inline ThreadedUnscaledFixedSystem threadedUnscaledFixedSystem;
class LateSystem final: protected ISystem {};
static constexpr inline LateSystem lateSystem;
class LateFixedSystem final: protected ISystem {};
static constexpr inline LateFixedSystem lateFixedSystem;
class LateUnscaledFixedSystem final: protected ISystem {};
static constexpr inline LateUnscaledFixedSystem lateUnscaledFixedSystem;

class SceneSystem final {};

template <typename T, typename... FutureTs, typename... PastTs>
[[nodiscard]] consteval auto impl_is_not_same_rec(With<PastTs...>) noexcept -> bool {
    const bool not_same = std::disjunction_v<std::disjunction<std::is_same<T, FutureTs>...>, std::disjunction<std::is_same<T, PastTs>...>>;
    if constexpr (!not_same && sizeof...(FutureTs) > 0) {
        return impl_is_not_same_rec<FutureTs...>(with<T, PastTs...>);
    }
    return not_same;
}

template <typename... Ts>
concept IsNotSameConcept = [] -> bool {
    if constexpr (sizeof...(Ts) > 0) {
        static_assert(!impl_is_not_same_rec<std::remove_cv<std::remove_reference<Ts>>...>(with<>), "Impossible de requeter des types en doublons");
    }
    return true;
}();

template <typename T>
concept IsNotEmptyConcept = [] -> bool {
    static_assert(!std::is_empty_v<T>, "Impossible de requeter un Marker (objet de taille 0)");
    return true;
}();

template <typename T>
concept IsFinalConcept = [] -> bool {
    static_assert(std::is_final_v<T>, "Impossible d'ajouter un composant non final (class *** final {})");
    return true;
}();

///////////////////////////////////////////////////////////////////////////////////

// class CompAny final {
// public:
//     constexpr CompAny() noexcept:
//         manager(nullptr) {
//     }

//     CompAny(const CompAny& other) {
//         if (!other.has_value()) {
//             manager = nullptr;
//         } else {
//             CompArg arg;
//             arg.any = this;
//             other.manager(OP_CLONE, &other, &arg);
//         }
//     }

//     CompAny(CompAny&& other) noexcept {
//         if (!other.has_value()) {
//             manager = nullptr;
//         } else {
//             CompArg arg;
//             arg.any = this;
//             other.manager(OP_TRANSFER, &other, &arg);
//         }
//     }

//     template <typename T> requires (std::copy_constructible<T>)
//     CompAny(T&& value):
//         data(new T{std::move(value)}),
//         manager(&compAnyManager<T>) {
//     }

//     template <typename T, typename... Args> requires (std::constructible_from<T, Args...>)
//     CompAny(std::in_place_type_t<T>, Args&&... args):
//         data(new T{std::forward<Args>(args)...}),
//         manager(&compAnyManager<T>) {
//     }

//     ~CompAny() {
//         reset();
//     }

//     CompAny& operator=(const CompAny& rhs) {
//         *this = CompAny(rhs);
//         return *this;
//     }

//     CompAny& operator=(CompAny&& rhs) noexcept {
//         if (!rhs.has_value()) {
//             reset();
//         } else if (this != &rhs) {
//             reset();
//             CompArg arg;
//             arg.any = this;
//             rhs.manager(OP_TRANSFER, &rhs, &arg);
//         }
//         return *this;
//     }

//     template <typename T> requires (std::copy_constructible<T>)
//     CompAny& operator=(T&& rhs) {
//         *this = CompAny(std::move(rhs));
//         return *this;
//     }

// public:
//     template <typename T, typename... Args> requires (std::constructible_from<T, Args...>)
//     T& emplace(Args&&... args) {
//         data = new T{std::forward<Args>(args)...};
//         manager = &compAnyManager<T>;
//         return *static_cast<T*>(data);
//     }

//     void swap(CompAny& rhs) noexcept {
//         if (!has_value() && !rhs.has_value()) {
//             return;
//         }

//         if (has_value() && rhs.has_value()) {
//             if (this == &rhs) {
//                 return;
//             }

//             CompAny tmp;
//             CompArg arg;
//             arg.any = &tmp;
//             rhs.manager(OP_TRANSFER, &rhs, &arg);
//             arg.any = &rhs;
//             manager(OP_TRANSFER, this, &arg);
//             arg.any = this;
//             tmp.manager(OP_TRANSFER, &tmp, &arg);
//         } else {
//             CompAny* empty = !has_value() ? this : &rhs;
//             CompAny* full = !has_value() ? &rhs : this;
//             CompArg arg;
//             arg.any = empty;
//             full->manager(OP_TRANSFER, full, &arg);
//         }
//     }

//     void reset() noexcept {
//         if (has_value()) {
//             manager(OP_DESTROY, this, nullptr);
//             manager = nullptr;
//         }
//     }

//     bool has_value() const {
//         return manager != nullptr;
//     }

//     const std::type_info& type() const noexcept {
//         if (!has_value()) {
//             return typeid(void);
//         }
//         CompArg arg;
//         manager(OP_TYPE, this, &arg);
//         return *arg.typeinfo;
//     }

// private:
//     enum CompOp: uint8_t {
//         OP_ACCESS, OP_TYPE, OP_CLONE, OP_DESTROY, OP_TRANSFER
//     };

//     union CompArg {
//         void* data;
//         const std::type_info* typeinfo;
//         CompAny* any;
//     };

// private:
//     template <typename T>
//     static void compAnyManager(CompOp op, const CompAny* any, CompArg* arg) {
//         auto ptr = static_cast<const T*>(any->data);
//         switch (op) {
//             case CompOp::OP_ACCESS:
//                 arg->data = const_cast<T*>(ptr);
//                 break;
//             case CompOp::OP_TYPE:
//                 arg->typeinfo = &typeid(T);
//                 break;
//             case CompOp::OP_CLONE:
//                 arg->any->data = new T{*ptr};
//                 arg->any->manager = any->manager;
//                 break;
//             case CompOp::OP_DESTROY:
//                 delete ptr;
//                 break;
//             case CompOp::OP_TRANSFER:
//                 arg->any->data = any->data;
//                 arg->any->manager = any->manager;
//                 const_cast<CompAny*>(any)->manager = nullptr;
//                 break;
//         }
//     }

// public:
//     template <typename T> requires (!std::same_as<T, void> && (std::copy_constructible<T> || std::is_rvalue_reference_v<T> || std::is_lvalue_reference_v<T>))
//     constexpr std::optional<T> comp_any_cast(CompAny& any) {
//         if (any.manager == &CompAny::compAnyManager<T> || any.type() == typeid(T)) {
//             CompArg arg;
//             any.manager(OP_ACCESS, &any, &arg);
//             return static_cast<T>(*arg.data);
//         }
//         return std::nullopt;
//     }

// private:
//     void* data;
//     void(*manager)(CompOp, const CompAny*, CompArg*);
// };

///////////////////////////////////////////////////////////////////////////////////

class CompPool final {
friend class Archetype;
friend class LiteArchetype;
friend class LateUpgrade;
public:
    [[nodiscard]] CompPool() noexcept = default;
    [[nodiscard]] CompPool(const Ent& entity, const std::any& component) noexcept:
        comps({{entity, component}}) {
    }

private:
    constexpr auto insert_entity(this auto& self, const Ent& entity, const std::any& component) noexcept -> void {
        self.comps.emplace(entity, component);
    }

    [[nodiscard]] constexpr auto get_entity(this auto& self, const Ent& entity) noexcept -> auto& {
        return self.comps.at(entity);
    }

    constexpr auto copy_entity(this auto& self, const Ent& entity, const CompPool& oth) noexcept -> void {
        self.comps.emplace(entity, oth.comps.at(entity));
    }

    constexpr auto remove_entity(this auto& self, const Ent& entity) noexcept -> void {
        self.comps.erase(entity);
    }

private:
    std::unordered_map<Ent, std::any> comps;
};

///////////////////////////////////////////////////////////////////////////////////

struct ArchetypeCreateWith final {};
constexpr ArchetypeCreateWith archetypeCreateWith;

struct ArchetypeCreateWithout final {};
constexpr ArchetypeCreateWithout archetypeCreateWithout;

class Archetype final {
friend class Registry;
friend class LiteArchetype;
friend class LateUpgrade;
template<typename... Ts>
friend class View;
public:
    Archetype() noexcept = default;

    Archetype(const Ent& ent, const std::unordered_map<Type, std::any>& components) noexcept:
        ents({ent}),
        pools(components.size()) {
        for (const auto& pair: components) {
            pools.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(pair.first),
                std::forward_as_tuple(ent, pair.second)
            );
        }
    }

    Archetype(ArchetypeCreateWith, Archetype& oldArch, const Ent& ent, const std::any& component) noexcept:
        ents({ent}),
        pools(oldArch.pools.size() + 1) {
        for (const auto& pair: oldArch.pools) {
            pools.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(pair.first),
                std::forward_as_tuple(ent, pair.second.comps.at(ent))
            );
        }
        pools.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(component.type().hash_code()),
            std::forward_as_tuple(ent, component)
        );
        oldArch.destroy(ent);
    }

    Archetype(ArchetypeCreateWithout, Archetype& oldArch, const Ent& ent, const Type& component_type) noexcept:
        ents({ent}),
        pools(oldArch.pools.size() - 1) {
        for (const auto& pair: oldArch.pools) {
            if (pair.first != component_type) {
                pools.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(pair.first),
                    std::forward_as_tuple(ent, pair.second.comps.at(ent))
                );
            }
        }
        oldArch.destroy(ent);
    }

private:
    constexpr auto new_entity(this auto& self, const Ent& entity, const std::unordered_map<Type, std::any>& components) noexcept -> void {
        self.ents.emplace(entity);
        for (const auto& pair: components) {
            self.pools.at(pair.first).insert_entity(entity, pair.second);
        }
    }

    constexpr auto add_component(this auto& self, const Ent& entity, Archetype& oldArch, const std::any& component) noexcept -> void {
        self.ents.emplace(entity);
        for (const auto& pair: oldArch.pools) {
            self.pools.at(pair.first).copy_entity(entity, pair.second);
        }
        self.pools.at(component.type().hash_code()).insert_entity(entity, component);
        oldArch.destroy(entity);
    }

    constexpr auto remove_component(this auto& self, const Ent& entity, Archetype& oldArch, const Type& component_type) noexcept -> void {
        self.ents.emplace(entity);
        for (const auto& pair: oldArch.pools) {
            if (pair.first != component_type) {
                self.pools.at(pair.first).copy_entity(entity, pair.second);
            }
        }
        oldArch.destroy(entity);
    }

    [[nodiscard]] constexpr auto get(this auto& self, const Ent& entity, const Type& component_type) noexcept -> auto&& {
        return self.pools.at(component_type).get_entity(entity);
    }

    [[nodiscard]] constexpr auto getTypes(this const auto& self, const Ent& entity) noexcept -> const std::vector<std::string> {
        std::vector<std::string> types;
        for (auto& pool: self.pools) {
            types.emplace_back(pool.second.comps.at(entity).type().name());
        }
        return types;
    }

    auto destroy(this auto& self, const Ent& entity) noexcept -> void {
        self.ents.erase(entity);
        for (auto& pool: self.pools) {
            pool.second.remove_entity(entity);
        }
    }

    [[nodiscard]] constexpr auto empty(this const auto& self) noexcept -> bool {
        return self.ents.empty();
    }

    [[nodiscard]] constexpr auto size(this const auto& self) noexcept -> std::size_t {
        return self.ents.size();
    }

    [[nodiscard]] constexpr auto containsType(this const auto& self, const Type type) noexcept -> bool {
        return self.pools.contains(type);
    }

private:
    template <typename... Ts>
    [[nodiscard]] constexpr auto getTupleWithEnt(this auto& self, const Ent& entity) noexcept -> std::tuple<const Ent&, Ts&...> {
        return std::forward_as_tuple(entity, std::any_cast<Ts&>(self.get(entity, typeid(Ts).hash_code()))...);
    }

private:
    [[nodiscard]] constexpr auto isTotalyCompatibleLate(this const auto& self, const std::unordered_map<Type, std::any>& components) noexcept -> bool {
        if (components.size() != self.pools.size()) {
            return false;
        }
        for (const auto& pair: components) {
            if (!self.pools.contains(pair.first)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr auto isTotalyCompatibleLate(this const auto& self, const Archetype& oldArch, const Type& component_type) noexcept -> bool {
        if (oldArch.pools.size() + 1 != self.pools.size() || !self.pools.contains(component_type)) {
            return false;
        }
        for (const auto& pair: oldArch.pools) {
            if (!self.pools.contains(pair.first)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr auto isTotalyCompatibleWithoutLate(this const auto& self, const Archetype& oldArch, const Type& component_type) noexcept -> bool {
        if (oldArch.pools.size() - 1 != self.pools.size() || self.pools.contains(component_type)) {
            return false;
        }
        for (const auto& pair: oldArch.pools) {
            if (!self.pools.contains(pair.first) && pair.first != component_type) {
                return false;
            }
        }
        return true;
    }

private:
    std::unordered_set<Ent> ents;
    std::unordered_map<Type, CompPool> pools;
};

///////////////////////////////////////////////////////////////////////////////////

template <typename... Ts>
class View final {
friend class Registry;
friend class LiteRegistry;
private:
    constexpr View(std::unordered_set<Archetype*>&& newArchs) noexcept:
        archs(std::move(newArchs)) {
    }

public:
    [[nodiscard]] constexpr auto empty() const noexcept -> bool {
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

    [[nodiscard]] constexpr auto size() const noexcept -> std::size_t {
        std::size_t newSize = 0;
        for (const auto* arch: archs) {
            newSize += arch->size();
        }
        return newSize;
    }

private:
    class ViewIterator final {
    friend class View;
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::tuple<const Ent, Ts&...>;
        using element_type = value_type;
        using pointer = value_type*;
        using reference = value_type&;
        using difference_type = std::ptrdiff_t;

    public:
        ViewIterator(const std::unordered_set<Archetype*>& newArchs, std::unordered_set<Archetype*>::const_iterator newArchsIt) noexcept:
            archsIt(newArchsIt),
            archs(newArchs) {
            if (archsIt != newArchs.end()) {
                entsIt = (*archsIt)->ents.begin();
            }
        }

        ViewIterator(const ViewIterator&) = default;
        ViewIterator(ViewIterator&&) = default;

        auto operator=(const ViewIterator& oth) -> ViewIterator& = delete;
        auto operator=(ViewIterator&& oth) -> ViewIterator& = delete;

        ~ViewIterator() = default;

        [[nodiscard]] constexpr auto operator *() const noexcept -> value_type {
            return (*archsIt)->getTupleWithEnt<Ts...>((*entsIt));
        }

        constexpr auto operator ++() noexcept -> ViewIterator& {
            entsIt++;
            if (entsIt == (*archsIt)->ents.end()) {
                archsIt++;
                if (archsIt != archs.end()) {
                    entsIt = (*archsIt)->ents.begin();
                }
            }
            return *this;
        }

        [[nodiscard]] friend constexpr auto operator !=(const ViewIterator& a, const ViewIterator& b) noexcept -> bool {
            return a.archsIt != b.archsIt;
        }

    private:
        std::unordered_set<Archetype*>::const_iterator archsIt;
        std::unordered_set<Ent>::iterator entsIt;
        const std::unordered_set<Archetype*>& archs;
    };

public:
    [[nodiscard]] constexpr auto begin() const noexcept -> ViewIterator{
        return {archs, archs.begin()};
    }

    [[nodiscard]] constexpr auto end() const noexcept -> ViewIterator {
        return {archs, archs.end()};
    }

private:
    const std::unordered_set<Archetype*> archs;
};

///////////////////////////////////////////////////////////////////////////////////

class Registry final {
friend class World;
friend class LateUpgrade;
private:
    Registry() noexcept:
        emptyArch(std::make_unique<Archetype>()) {
    }

private:
    ~Registry() noexcept {
        for (auto* arch: archs) {
            delete arch;
        }
    }

    [[nodiscard]] constexpr auto getEntToken() noexcept -> Ent {
        Ent ent = lastEnt++;

        if (!entTokens.empty()) {
            lastEnt--;
            ent = entTokens.back();
            entTokens.pop_back();
        }

        entArch.emplace(ent, emptyArch.get());

        return ent;
    }

    auto newEnt(const Ent& entity, const std::unordered_map<Type, std::any>& components) noexcept -> void {
        auto entArchIt = entArch.find(entity);
        if (entArchIt->second->size() > 0) {
            std::println("ZerEngine: Impossible d'inserer une entité deja existante - [{}]", entity);
            return;
        }

        std::unordered_set<Archetype*> compatiblesArchs(archs);
        for (const auto& pairAny: components) {
            filterArchsByType(pairAny.first, compatiblesArchs);
        }
        for (auto* arch: compatiblesArchs) {
            if (arch->isTotalyCompatibleLate(components)) {
                arch->new_entity(entity, components);
                entArchIt->second = arch;
                return;
            }
        }

        auto* arch = new Archetype(entity, components);
        archs.emplace(arch);
        entArchIt->second = arch;
        for (const auto& pair: arch->pools) {
            emplaceArchByType(pair.first, arch);
        }
    }

    auto add(const Ent& entity, const std::any& component) noexcept -> void {
        auto entArchIt = entArch.find(entity);
        if (entArchIt == entArch.end()) {
            std::println("ZerEngine: Impossible d'ajouter un composant sur une entité inexistante - [{}]", entity);
            return;
        }

        if (entArchIt->second->containsType(component.type().hash_code())) {
            std::println("ZerEngine: Impossible d'ajouter 2 composants identiques  - [{}] - {}", entity, component.type().name());
            return;
        }

        auto* oldArch = entArchIt->second;

        std::unordered_set<Archetype*> compatiblesArchs(archs);
        for (const auto& pairPools: oldArch->pools) {
            filterArchsByType(pairPools.first, compatiblesArchs);
        }
        filterArchsByType(component.type().hash_code(), compatiblesArchs);
        for (auto* arch: compatiblesArchs) {
            if (arch->isTotalyCompatibleLate(*oldArch, component.type().hash_code())) {
                arch->add_component(entity, *oldArch, component);
                entArchIt->second = arch;
                emplaceArchByType(component.type().hash_code(), arch);
                removeOldArchIfEmpty(oldArch);
                return;
            }
        }

        auto* arch = new Archetype(archetypeCreateWith, *oldArch, entity, component);
        archs.emplace(arch);
        entArchIt->second = arch;
        for (const auto& pair: arch->pools) {
            emplaceArchByType(pair.first, arch);
        }
        removeOldArchIfEmpty(oldArch);
    }

    auto remove(const Ent& entity, const Type& component_type) noexcept -> void {
        auto entArchIt = entArch.find(entity);
        if (entArchIt == entArch.end()) {
            std::println("ZerEngine: Impossible de supprimer un composant sur une entite inexistante - [{}]", entity);
            return;
        }

        if (!entArchIt->second->containsType(component_type)) {
            std::println("ZerEngine: Impossible de supprimer un composant qui n'existe pas - [{}]", entity);
            return;
        }

        auto* oldArch = entArchIt->second;

        std::unordered_set<Archetype*> compatiblesArchs(archs);
        for (const auto& pairPools: oldArch->pools) {
            if (pairPools.first != component_type) {
                filterArchsByType(pairPools.first, compatiblesArchs);
            }
        }
        for (auto* arch: compatiblesArchs) {
            if (arch->isTotalyCompatibleWithoutLate(*oldArch, component_type)) {
                arch->remove_component(entity, *oldArch, component_type);
                entArchIt->second = arch;
                removeOldArchIfEmpty(oldArch);
                return;
            }
        }

        auto* arch = new Archetype(archetypeCreateWithout, *oldArch, entity, component_type);
        archs.emplace(arch);
        entArchIt->second = arch;
        for (const auto& pair: arch->pools) {
            emplaceArchByType(pair.first, arch);
        }
        removeOldArchIfEmpty(oldArch);
    }

    [[nodiscard]] constexpr auto exist(const Ent& entity) const noexcept -> bool {
        return entArch.contains(entity);
    }

    [[nodiscard]] auto has(const Ent& entity, const std::initializer_list<Type>& types) const noexcept -> bool {
        auto entArchIt = entArch.find(entity);
        if (entArchIt == entArch.end()) {
            return false;
        }
        for (const auto type: types) {
            if (!entArchIt->second->pools.contains(type)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr auto get(this auto& self, const Ent& entity, const Type& component_type) noexcept -> auto&& {
        return self.entArch.at(entity)->get(entity, component_type);
    }

    constexpr auto getTypes(const Ent& entity) const noexcept -> const std::vector<std::string> {
        return entArch.at(entity)->getTypes(entity);
    }

    auto destroy(const Ent& entity) noexcept -> void {
        auto entArchIt = entArch.find(entity);
        if (entArchIt == entArch.end()) {
            std::println("ZerEngine: Impossible de detruire une entitée qui n'existe pas");
            return;
        }

        auto* arch = entArchIt->second;
        arch->destroy(entity);
        removeOldArchIfEmpty(arch);
        entArch.erase(entArchIt);
        entTokens.push_back(entity);
        detachChildren(entity);
        removeParent(entity);
    }

    auto clean() noexcept -> void {
        for (auto* arch: archs) {
            delete arch;
        }

        emptyArch = std::make_unique<Archetype>();
        lastEnt = 1;
        entTokens.clear();
        archs.clear();
        entArch.clear();
        archsByType.clear();
        parentChildrens.clear();
        childrenParent.clear();
    }

private:
    constexpr auto appendChildrenInactiveRecDown(const Ent& parentEntity) noexcept -> void {
        if (auto childrenOpt = getChildren(parentEntity)) {
            for (auto childEnt: childrenOpt.value().get()) {
                if (!has(childEnt, {typeid(IsInactive).hash_code()})) {
                    add(childEnt, std::make_any<IsInactive>());
                }
                appendChildrenInactiveRecDown(childEnt);
            }
        }
    }

    constexpr auto appendChildrenInactiveRecUp(const Ent& parentEntity) noexcept -> void {
        if (has(parentEntity, {typeid(IsInactive).hash_code()})) {
            if (auto childrenOpt = getChildren(parentEntity)) {
                for (auto childEnt: childrenOpt.value().get()) {
                    if (!has(childEnt, {typeid(IsInactive).hash_code()})) {
                        add(childEnt, std::make_any<IsInactive>());
                    }
                    appendChildrenInactiveRecDown(childEnt);
                }
            }
            return;
        }

        if (auto parentOpt = getParent(parentEntity)) {
            appendChildrenInactiveRecUp(parentOpt.value());
        }
    }

    constexpr auto appendChildrenDontDestroyOnLoadRecDown(const Ent& parentEntity) noexcept -> void {
        if (auto childrenOpt = getChildren(parentEntity)) {
            for (auto childEnt: childrenOpt.value().get()) {
                if (!has(childEnt, {typeid(DontDestroyOnLoad).hash_code()})) {
                    add(childEnt, std::make_any<DontDestroyOnLoad>());
                }
                appendChildrenDontDestroyOnLoadRecDown(childEnt);
            }
        }
    }

    constexpr auto appendChildrenDontDestroyOnLoadRecUp(const Ent& parentEntity) noexcept -> void {
        if (has(parentEntity, {typeid(DontDestroyOnLoad).hash_code()})) {
            if (auto childrenOpt = getChildren(parentEntity)) {
                for (auto childEnt: childrenOpt.value().get()) {
                    if (!has(childEnt, {typeid(DontDestroyOnLoad).hash_code()})) {
                        add(childEnt, std::make_any<DontDestroyOnLoad>());
                    }
                    appendChildrenDontDestroyOnLoadRecDown(childEnt);
                }
            }
            return;
        }

        if (auto parentOpt = getParent(parentEntity)) {
            appendChildrenDontDestroyOnLoadRecUp(parentOpt.value());
        }
    }

    auto appendChildren(const Ent& parentEntity, const std::unordered_set<Ent>& childrenEnt) noexcept -> void {
        auto parentIt = parentChildrens.find(parentEntity);
        if (parentIt == parentChildrens.end()) {
            parentIt = parentChildrens.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(parentEntity),
                std::forward_as_tuple()
            ).first;
        }
        for (const auto childEnt: childrenEnt) {
            if (childrenParent.contains(childEnt)) {
                std::println("Children: Tu ne peux pas avoir deux parents Billy[{}]", childEnt);
            } else if (parentEntity == childEnt) {
                std::println("Children: Impossible d'etre son propre pere");
            } else {
                childrenParent.emplace(childEnt, parentEntity);
                parentIt->second.emplace(childEnt);
            }
        }
        if (parentIt->second.empty()) {
            parentChildrens.erase(parentIt);
        }
        appendChildrenInactiveRecUp(parentEntity);
        appendChildrenDontDestroyOnLoadRecUp(parentEntity);
    }

    auto appendChildren(const Ent& parentEntity, const std::vector<Ent>& childrenEnt) noexcept -> void {
        auto parentIt = parentChildrens.find(parentEntity);
        if (parentIt == parentChildrens.end()) {
            parentIt = parentChildrens.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(parentEntity),
                std::forward_as_tuple()
            ).first;
        }
        for (const auto childEnt: childrenEnt) {
            if (childrenParent.contains(childEnt)) {
                std::println("Children: Tu ne peux pas avoir deux parents Billy[{}]", childEnt);
            } else if (parentEntity == childEnt) {
                std::println("Children: Impossible d'etre son propre pere");
            } else {
                childrenParent.emplace(childEnt, parentEntity);
                parentIt->second.emplace(childEnt);
            }
        }
        if (parentIt->second.empty()) {
            parentChildrens.erase(parentIt);
        }
        appendChildrenInactiveRecUp(parentEntity);
        appendChildrenDontDestroyOnLoadRecUp(parentEntity);
    }

    auto detachChildren(const Ent& parentEntity) noexcept -> void {
        if (auto parentIt = parentChildrens.find(parentEntity); parentIt != parentChildrens.end()) {
            for (const auto childEnt: parentIt->second) {
                if (auto childrenIt = childrenParent.find(childEnt); childrenIt != childrenParent.end()) {
                    childrenParent.erase(childrenIt);
                }
            }
            parentChildrens.erase(parentIt);
        }
    }

    auto removeParent(const Ent& childEntity) noexcept -> void {
        if (auto childrenIt = childrenParent.find(childEntity); childrenIt != childrenParent.end()) {
            if (auto parentIt = parentChildrens.find(childrenIt->second); parentIt != parentChildrens.end()) {
                parentIt->second.erase(childEntity);
                if (parentIt->second.empty()) {
                    parentChildrens.erase(parentIt);
                }
            }
            childrenParent.erase(childrenIt);
        }
    }

    [[nodiscard]] constexpr auto hasChildren(const Ent& parentEntity) const noexcept -> bool {
        return parentChildrens.contains(parentEntity);
    }

    [[nodiscard]] auto getChildren(const Ent& parentEntity) const noexcept -> std::optional<std::reference_wrapper<const std::unordered_set<Ent>>> {
        if (auto parentIt = parentChildrens.find(parentEntity); parentIt != parentChildrens.end()) {
            return std::make_optional<std::reference_wrapper<const std::unordered_set<Ent>>>(std::reference_wrapper<const std::unordered_set<Ent>>(parentIt->second));
        }
        return std::nullopt;
    }

    [[nodiscard]] constexpr auto hasParent(const Ent& childEntity) const noexcept -> bool {
        return childrenParent.contains(childEntity);
    }

    [[nodiscard]] auto getParent(const Ent& childEntity) const noexcept -> std::optional<Ent> {
        if (auto childIt = childrenParent.find(childEntity); childIt != childrenParent.end()) {
            return childIt->second;
        }
        return std::nullopt;
    }

private:
    template <typename... Comps>
    [[nodiscard]] constexpr auto view(const std::initializer_list<Type>& compFilterTypes, const std::initializer_list<Type>& excludeTypes) const noexcept -> const View<Comps...> {
        std::unordered_set<Archetype*> internalArchs;
        if (compFilterTypes.size() > 0) {
            viewAddComp(internalArchs, compFilterTypes);
        } else {
            internalArchs = archs;
        }
        if (excludeTypes.size() > 0) {
            viewWithoutComp(internalArchs, excludeTypes);
        }
        return {std::move(internalArchs)};
    }

private:
    constexpr auto viewAddComp(std::unordered_set<Archetype*>& internalArchs, const std::initializer_list<Type>& compTypes) const noexcept -> void {
        std::size_t i = 0;
        for (const auto compType: compTypes) {
            if (i == 0) {
                if (auto archsByTypeIt = archsByType.find(compType); archsByTypeIt != archsByType.end()) {
                    internalArchs = archsByTypeIt->second;
                } else {
                    internalArchs.clear();
                    return;
                }
                i++;
            } else {
                if (auto archsByTypeIt = archsByType.find(compType); archsByTypeIt != archsByType.end()) {
                    const auto& othArchs = archsByTypeIt->second;
                    std::vector<Archetype*> toErase;
                    for (auto* arch: internalArchs) {
                        if (!othArchs.contains(arch)) {
                            toErase.emplace_back(arch);
                        }
                    }
                    for (auto* arch: toErase) {
                        internalArchs.erase(arch);
                    }
                } else {
                    internalArchs.clear();
                    return;
                }
            }
        }
    }

    constexpr auto viewWithoutComp(std::unordered_set<Archetype*>& internalArchs, const std::initializer_list<Type>& compTypes) const noexcept -> void {
        for (const auto compType: compTypes) {
            if (auto archsByTypeIt = archsByType.find(compType); archsByTypeIt != archsByType.end()) {
                for (auto* arch: archsByTypeIt->second) {
                    if (internalArchs.contains(arch)) {
                        internalArchs.erase(arch);
                    }
                }
            }
        }
    }

    auto filterArchsByType(const Type& component_type, std::unordered_set<Archetype*>& compatibleArchs) noexcept -> void {
        std::unordered_set<Archetype*> newArchs;
        if (auto archsByTypeIt = archsByType.find(component_type); archsByTypeIt != archsByType.end()) {
            for (auto* arch: archsByTypeIt->second) {
                if (compatibleArchs.contains(arch)) {
                    newArchs.emplace(arch);
                }
            }
        }
        compatibleArchs = newArchs;
    }

private:
    auto emplaceArchByType(const Type& component_type, Archetype* arch) noexcept -> void {
        if (auto archsByTypeIt = archsByType.find(component_type); archsByTypeIt != archsByType.end()) {
            archsByTypeIt->second.emplace(arch);
        } else {
            archsByType.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(component_type),
                std::forward_as_tuple(std::initializer_list<Archetype*>{arch})
            );
        }
    }

    constexpr auto removeOldArchIfEmpty(Archetype* oldArch) noexcept -> void {
        if (oldArch->size() <= 0 && oldArch != emptyArch.get()) {
            for (const auto& pair: oldArch->pools) {
                archsByType.at(pair.first).erase(oldArch);
            }
            delete oldArch;
            archs.erase(oldArch);
        }
    }

private:
    Ent lastEnt = 1;
    std::vector<Ent> entTokens;
    std::unique_ptr<Archetype> emptyArch;
    std::unordered_set<Archetype*> archs;
    std::unordered_map<Ent, Archetype*> entArch;
    std::unordered_map<Type, std::unordered_set<Archetype*>> archsByType;
    std::unordered_map<Ent, std::unordered_set<Ent>> parentChildrens;
    std::unordered_map<Ent, Ent> childrenParent;
};

///////////////////////////////////////////////////////////////////////////////////

class World;

class LateUpgrade final {
friend class World;
private:
    LateUpgrade() = default;

private:
    auto newEnt(const Ent& ent, const std::initializer_list<std::pair<const Type, std::any>>& newList) noexcept -> Ent {
        const std::unique_lock<std::mutex> lock(mtx);
        addEnts.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(ent),
            std::forward_as_tuple(newList)
        );
        return ent;
    }

    void add(const Registry& reg, const Ent& ent, const std::initializer_list<std::pair<const Type, std::any>>& newList) noexcept {
        const std::unique_lock<std::mutex> lock(mtx);
        auto addCompsIt = addComps.find(ent);
        if (addCompsIt != addComps.end()) {
            for (const auto& pair: newList) {
                if (!addCompsIt->second.contains(pair.first)) {
                    addCompsIt->second.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(pair.first),
                        std::forward_as_tuple(pair.second)
                    );
                } else {
                    std::println("LateUpgrade: No Add Sur Comp: Le Composant {} existe deja (dans addComp)", pair.second.type().name());
                }
            }
        } else {
            addCompsIt = addComps.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(ent),
                std::forward_as_tuple()
            ).first;
            for (const auto& pair: newList) {
                if (!reg.has(ent, {pair.first})) {
                    if (!addCompsIt->second.contains(pair.first)) {
                        addCompsIt->second.emplace(
                            std::piecewise_construct,
                            std::forward_as_tuple(pair.first),
                            std::forward_as_tuple(pair.second)
                        );
                    } else {
                        std::println("LateUpgrade: No Add Sur Comp: Le Composant {} existe deja (en double sur l'ajout dans addComp)", pair.second.type().name());
                    }
                } else {
                    std::println("LateUpgrade: No Add Sur Comp: Le Composant {} existe deja (sur l'entite)", pair.second.type().name());
                }
            }
        }
    }

    void remove(const Ent& ent, const Type& type) noexcept {
        const std::unique_lock<std::mutex> lock(mtx);
        if (!delEnts.contains(ent)) {
            auto addEntsIt = addEnts.find(ent);
            if (addEntsIt == addEnts.end() || !addEntsIt->second.contains(type)) {
                auto addCompsIt = addComps.find(ent);
                if (addCompsIt == addComps.end() || !addCompsIt->second.contains(type)) {
                    auto delCompsIt = delComps.find(ent);
                    if (delCompsIt == delComps.end()) {
                        delComps.emplace(
                            std::piecewise_construct,
                            std::forward_as_tuple(ent),
                            std::forward_as_tuple(
                                std::initializer_list<Type>{
                                    type
                                }
                            )
                        );
                    } else {
                        delCompsIt->second.emplace(
                            type
                        );
                    }
                } else {
                    addCompsIt->second.erase(type);
                }
            } else {
                addEntsIt->second.erase(type);
            }
        }
    }

    void appendChildren(const Ent& parentEnt, const std::vector<Ent>& childrenEnt) {
        const std::unique_lock<std::mutex> lock(mtx);
        if (auto addParentChildrenIt = addParentChildren.find(parentEnt); addParentChildrenIt != addParentChildren.end()) {
            for (const auto childEnt: childrenEnt) {
                addParentChildrenIt->second.emplace(childEnt);
            }
        } else {
            addParentChildren.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(parentEnt),
                std::forward_as_tuple(childrenEnt.begin(), childrenEnt.end())
            );
        }
    }

    void setActive(const Ent& ent) {
        setActiveEnts.emplace(ent);
    }

    void setInactive(const Ent& ent) {
        setInactiveEnts.emplace(ent);
    }

    void addDontDestroyOnLoad(const Ent& ent) {
        addDontDestroyOnLoadEnts.emplace(ent);
    }

    void destroyChildRec(Registry& reg, const Ent& parentEnt) noexcept {
        if (reg.exist(parentEnt)) {
            if (auto childrenOpt = reg.getChildren(parentEnt)) {
                const std::unordered_set<Ent> copyChildrenSet = childrenOpt.value().get();
                for (const auto childEnt: copyChildrenSet) {
                    destroyChildRec(reg, childEnt);
                }
            }
            reg.destroy(parentEnt);
        }
    }

    void destroy(const Ent& ent) noexcept {
        const std::unique_lock<std::mutex> lock(mtx);
        delEnts.emplace(ent);
    }

    void loadScene(std::function<void(SceneSystem, World&)>&& newScene) noexcept {
        needClean = true;
        newSceneFunc = std::move(newScene);
    }

private:
    auto getTypes(const Ent& ent) noexcept -> const std::vector<std::string> {
        const std::unique_lock<std::mutex> lock(mtx);
        std::vector<std::string> types;
        for (const auto& pairType: addEnts.at(ent)) {
            types.emplace_back(pairType.second.type().name());
        }
        for (const auto& pairType: addComps.at(ent)) {
            types.emplace_back(pairType.second.type().name());
        }
        return types;
    }

private:
    void setActiveRec(Registry& reg, const Ent& ent) {
        if (reg.has(ent, {typeid(IsInactive).hash_code()})) {
            reg.remove(ent, typeid(IsInactive).hash_code());
            if (auto childrenOpt = reg.getChildren(ent)) {
                for (auto childEnt: childrenOpt.value().get()) {
                    setActiveRec(reg, childEnt);
                }
            }
        }
    }

    void setInactiveRec(Registry& reg, const Ent& ent) {
        if (!reg.has(ent, {typeid(IsInactive).hash_code()})) {
            reg.add(ent, std::make_any<IsInactive>());
            if (auto childrenOpt = reg.getChildren(ent)) {
                for (auto childEnt: childrenOpt.value().get()) {
                    setInactiveRec(reg, childEnt);
                }
            }
        }
    }

    void addDontDetroyOnLoadRec(Registry& reg, const Ent& ent) {
        if (!reg.has(ent, {typeid(DontDestroyOnLoad).hash_code()})) {
            reg.add(ent, std::make_any<DontDestroyOnLoad>());
            if (auto childrenOpt = reg.getChildren(ent)) {
                for (auto childEnt: childrenOpt.value().get()) {
                    addDontDetroyOnLoadRec(reg, childEnt);
                }
            }
        }
    }

private:
    void upgrade(World& world, Registry& reg) noexcept {
        for (const auto& pair: addEnts) {
            reg.newEnt(pair.first, pair.second);
        }

        for (const auto& pair: delComps) {
            for (const auto& type: pair.second) {
                reg.remove(pair.first, type);
            }
        }

        for (const auto& pair: addComps) {
            for (const auto& pairType: pair.second) {
                reg.add(pair.first, pairType.second);
            }
        }

        for (const auto& pair: addParentChildren) {
            reg.appendChildren(pair.first, pair.second);
        }

        for (const auto ent: setInactiveEnts) {
            setInactiveRec(reg, ent);
        }

        for (const auto ent: addDontDestroyOnLoadEnts) {
            addDontDetroyOnLoadRec(reg, ent);
        }

        for (const Ent ent: delEnts) {
            destroyChildRec(reg, ent);
        }

        for (const auto ent: setActiveEnts) {
            setActiveRec(reg, ent);
        }

        delEnts.clear();
        addEnts.clear();
        delComps.clear();
        addComps.clear();
        setActiveEnts.clear();
        setInactiveEnts.clear();
        addDontDestroyOnLoadEnts.clear();
        addParentChildren.clear();

        if (needClean) {
            needClean = false;
            std::unordered_map<Ent, std::unordered_set<Ent>> dontDestroyesHierarchies;
            for (auto [dontDestroyEnt]: reg.view({typeid(DontDestroyOnLoad).hash_code()}, {})) {
                auto* arch = reg.entArch.at(dontDestroyEnt);
                std::unordered_map<Type, std::any> comps;
                for (auto& pair: arch->pools) {
                    comps.emplace(pair.first, pair.second.comps.at(dontDestroyEnt));
                }
                dontDestroyes.emplace(dontDestroyEnt, comps);
                if (auto childrenIt = reg.parentChildrens.find(dontDestroyEnt); childrenIt != reg.parentChildrens.end()) {
                    dontDestroyesHierarchies.emplace(dontDestroyEnt, childrenIt->second);
                }
            }
            reg.clean();

            std::unordered_map<Ent, Ent> oldToNewEnts;
            for (const auto& pair: dontDestroyes) {
                auto newEntId = reg.getEntToken();
                reg.newEnt(newEntId, pair.second);
                oldToNewEnts.emplace(pair.first, newEntId);
            }
            dontDestroyes.clear();

            for (const auto& pair: dontDestroyesHierarchies) {
                auto newEntId = oldToNewEnts.at(pair.first);
                std::unordered_set<Ent> newChildrens;
                for (auto oldChildEnt: pair.second) {
                    if (auto oldToNewEntsIt = oldToNewEnts.find(oldChildEnt); oldToNewEntsIt != oldToNewEnts.end()) {
                        newChildrens.emplace(oldToNewEntsIt->second);
                    } else {
                        std::println("Probleme sur la purge des scenes");
                    }
                }
                reg.appendChildren(newEntId, newChildrens);
            }

            newSceneFunc({}, world);
        }
    }

private:
    std::mutex mtx;
    std::unordered_map<Ent, std::unordered_map<Type, std::any>> addEnts;
    std::unordered_map<Ent, std::unordered_map<Type, std::any>> addComps;
    std::unordered_set<Ent> delEnts;
    std::unordered_map<Ent, std::unordered_set<Type>> delComps;
    std::unordered_map<Ent, std::unordered_map<Type, std::any>> dontDestroyes;

    std::unordered_map<Ent, std::unordered_set<Ent>> addParentChildren;

    std::unordered_set<Ent> setActiveEnts;
    std::unordered_set<Ent> setInactiveEnts;

    std::unordered_set<Ent> addDontDestroyOnLoadEnts;

    bool needClean = false;
    std::function<void(SceneSystem, World&)> newSceneFunc;
};

///////////////////////////////////////////////////////////////////////////////////

class TypeMap final {
friend class World;
friend class ZerEngine;
private:
    constexpr void emplace(const Type&& type, const std::any&& any) noexcept {
        typeMap.emplace(std::move(type), std::move(any));
    }

    [[nodiscard]] constexpr auto get(this auto& self, const Type& type) noexcept -> auto& {
        return self.typeMap.at(type);
    }

    constexpr void clear() noexcept {
        typeMap.clear();
    }

private:
    std::unordered_map<std::size_t, std::any> typeMap;
};

///////////////////////////////////////////////////////////////////////////////////

class ThreadedFixedSet {
friend class Sys;
public:
    ThreadedFixedSet(std::initializer_list<ThreadedFixedSet>&& newSubSets):
        condition(nullptr),
        tasks(),
        subSets(std::move(newSubSets)) {
    }

    ThreadedFixedSet(std::function<bool(World&)>&& newCondition, std::initializer_list<ThreadedFixedSet>&& newSubSets):
        condition(std::move(newCondition)),
        tasks(),
        subSets(std::move(newSubSets)) {
    }

    ThreadedFixedSet(std::initializer_list<std::function<void(ThreadedFixedSystem, World&)>>&& newTasks, std::initializer_list<ThreadedFixedSet>&& newSubSets = {}):
        condition(nullptr),
        tasks(std::move(newTasks)),
        subSets(std::move(newSubSets)) {
    }

    ThreadedFixedSet(std::function<bool(World&)>&& newCondition, std::initializer_list<std::function<void(ThreadedFixedSystem, World&)>>&& newTasks, std::initializer_list<ThreadedFixedSet>&& newSubSets = {}):
        condition(std::move(newCondition)),
        tasks(std::move(newTasks)),
        subSets(std::move(newSubSets)) {
    }

private:
    std::function<bool(World&)> condition;
    std::vector<std::function<void(ThreadedFixedSystem, World&)>> tasks;
    std::vector<ThreadedFixedSet> subSets;
};

///////////////////////////////////////////////////////////////////////////////////

class World;

class ThreadPool final {
friend class Sys;
private:
    ThreadPool(World& newWorld, std::size_t newNbThreads) noexcept:
        world(newWorld),
        nbThreads(newNbThreads) {
        for (std::size_t i = 0; i < nbThreads; i++) {
            threads.emplace_back([this] {
                task();
            });
        }
    }

private:
    ~ThreadPool() noexcept {
        stop();
        for (auto& thread: threads) {
            thread.join();
        }
    }

    void addTasks(const std::vector<std::function<void(ThreadedSystem, World&)>>& newTasks) noexcept {
        tasks.emplace_back(newTasks);
    }

    void addFixedTasks(const std::vector<std::function<void(ThreadedFixedSystem, World&)>>& newTasks) noexcept {
        fixedTasks.emplace_back(newTasks);
    }

    void addUnscaledFixedTasks(const std::vector<std::function<void(ThreadedUnscaledFixedSystem, World&)>>& newTasks) noexcept {
        unscaledFixedTasks.emplace_back(newTasks);
    }

    void run() noexcept {
        if (!tasks.empty()) {
            nbTasksDone = tasks[0].size();
            cvTask.notify_all();
        } else {
            nbTasksDone = 0;
        }
    }

    void fixedRun() noexcept {
        if (!fixedTasks.empty()) {
            nbTasksDone = fixedTasks[0].size();
            cvTask.notify_all();
        } else {
            nbTasksDone = 0;
        }
    }

    void unscaledFixedRun() noexcept {
        if (!unscaledFixedTasks.empty()) {
            nbTasksDone = unscaledFixedTasks[0].size();
            cvTask.notify_all();
        } else {
            nbTasksDone = 0;
        }
    }

    void wait() noexcept {
        std::unique_lock<std::mutex> lock(mtx);
        cvFinished.wait(lock, [&]() {
            if (!tasks.empty() && nbTasksDone != 0) {
               cvTask.notify_all();
            }
            return (tasks.empty() && (nbTasks == 0));
        });
    }

    void fixedWait() noexcept {
        std::unique_lock<std::mutex> lock(mtx);
        cvFinished.wait(lock, [&]() {
            if (!fixedTasks.empty() && nbTasksDone != 0) {
               cvTask.notify_all();
            }
            return (fixedTasks.empty() && (nbTasks == 0));
        });
    }

    void unscaledFixedWait() noexcept {
        std::unique_lock<std::mutex> lock(mtx);
        cvFinished.wait(lock, [&]() {
            if (!unscaledFixedTasks.empty() && nbTasksDone != 0) {
               cvTask.notify_all();
            }
            return (unscaledFixedTasks.empty() && (nbTasks == 0));
        });
    }

private:
    void task() noexcept {
        std::srand(std::time(nullptr));
        std::unique_lock<std::mutex> lock(mtx);
        while (true) {
            cvTask.wait(lock, [&]() {
                return (nbTasks < nbThreads) && (((!tasks.empty() || !fixedTasks.empty()) && nbTasksDone != 0) || isStop);
            });
            if (isStop && tasks.empty() && fixedTasks.empty()) {
                return;
            }

            if (!tasks.empty()) {
                nbTasks++;
                auto newTask = tasks[0].back();
                tasks[0].pop_back();
                nbTasksDone--;
                lock.unlock();

                newTask({}, world);

                lock.lock();
                nbTasks--;

                if (nbTasksDone == 0 && nbTasks == 0) {
                    tasks.erase(tasks.begin());
                    if (!tasks.empty()) {
                        nbTasksDone = tasks[0].size();
                        cvFinished.notify_one();
                    }
                }

                if (tasks.empty() && nbTasks == 0) {
                    cvFinished.notify_one();
                }
            } else if (!fixedTasks.empty()) {
                nbTasks++;
                auto newTask = fixedTasks[0].back();
                fixedTasks[0].pop_back();
                nbTasksDone--;
                lock.unlock();

                newTask({}, world);

                lock.lock();
                nbTasks--;

                if (nbTasksDone == 0 && nbTasks == 0) {
                    fixedTasks.erase(fixedTasks.begin());
                    if (!fixedTasks.empty()) {
                        nbTasksDone = fixedTasks[0].size();
                        cvFinished.notify_one();
                    }
                }

                if (fixedTasks.empty() && nbTasks == 0) {
                    cvFinished.notify_one();
                }
            } else if (!unscaledFixedTasks.empty()) {
                nbTasks++;
                auto newTask = unscaledFixedTasks[0].back();
                unscaledFixedTasks[0].pop_back();
                nbTasksDone--;
                lock.unlock();

                newTask({}, world);

                lock.lock();
                nbTasks--;

                if (nbTasksDone == 0 && nbTasks == 0) {
                    unscaledFixedTasks.erase(unscaledFixedTasks.begin());
                    if (!unscaledFixedTasks.empty()) {
                        nbTasksDone = unscaledFixedTasks[0].size();
                        cvFinished.notify_one();
                    }
                }

                if (unscaledFixedTasks.empty() && nbTasks == 0) {
                    cvFinished.notify_one();
                }
            }
        }
    }

    void stop() noexcept {
        const std::unique_lock<std::mutex> lock(mtx);
        isStop = true;
        cvTask.notify_all();
    }

private:
    World& world;
    std::vector<std::vector<std::function<void(ThreadedSystem, World&)>>> tasks;
    std::vector<std::vector<std::function<void(ThreadedFixedSystem, World&)>>> fixedTasks;
    std::vector<std::vector<std::function<void(ThreadedUnscaledFixedSystem, World&)>>> unscaledFixedTasks;
    std::mutex mtx;
    std::size_t nbTasksDone {0};
    std::condition_variable cvTask;
    std::condition_variable cvFinished;
    std::vector<std::thread> threads;
    std::size_t nbTasks {0};
    std::size_t nbThreads;
    bool isStop {false};
};

///////////////////////////////////////////////////////////////////////////////////

class Sys final {
friend class World;
friend class ZerEngine;
private:
    Sys(World& world) noexcept:
        threadpool(world, std::thread::hardware_concurrency() - 1)
    {
        std::srand(std::time(nullptr));
    }

private:
    constexpr void useMultithreading(bool newVal) noexcept {
        isUseMultithreading = newVal;
    }

    constexpr void addStartSys(std::function<void(StartSystem, World&)>&& func) noexcept {
        startSystems.emplace_back(std::move(func));
    }

    constexpr void addMainCondSys(std::function<bool(World&)>&& cond, std::initializer_list<std::function<void(MainSystem, World&)>>&& funcs) noexcept {
        mainSystems.emplace_back(std::move(cond), std::move(funcs));
    }

    constexpr void addMainFixedCondSys(std::function<bool(World&)>&& cond, std::initializer_list<std::function<void(MainFixedSystem, World&)>>&& funcs) noexcept {
        mainFixedSystems.emplace_back(std::move(cond), std::move(funcs));
    }

    constexpr void addMainUnscaledFixedCondSys(std::function<bool(World&)>&& cond, std::initializer_list<std::function<void(MainUnscaledFixedSystem, World&)>>&& funcs) noexcept {
        mainUnscaledFixedSystems.emplace_back(std::move(cond), std::move(funcs));
    }

    constexpr void addThreadedCondSys(std::function<bool(World&)>&& cond, std::initializer_list<std::function<void(ThreadedSystem, World&)>>&& funcs) noexcept {
        threadedSystems.emplace_back(std::move(cond), std::move(funcs));
    }

    constexpr void addThreadedFixedCondSys(const ThreadedFixedSet& newSet) noexcept {
        threadedFixedSystems.emplace_back(newSet);
    }

    constexpr void addThreadedUnscaledFixedCondSys(std::function<bool(World&)>&& cond, std::initializer_list<std::function<void(ThreadedUnscaledFixedSystem, World&)>>&& funcs) noexcept {
        threadedUnscaledFixedSystems.emplace_back(std::move(cond), std::move(funcs));
    }

    constexpr void addLateCondSys(std::function<bool(World&)>&& cond, std::initializer_list<std::function<void(LateSystem, World&)>>&& funcs) noexcept {
        lateSystems.emplace_back(std::move(cond), std::move(funcs));
    }

    constexpr void addLateFixedCondSys(std::function<bool(World&)>&& cond, std::initializer_list<std::function<void(LateFixedSystem, World&)>>&& funcs) noexcept {
        lateFixedSystems.emplace_back(std::move(cond), funcs);
    }

    constexpr void addLateUnscaledFixedCondSys(std::function<bool(World&)>&& cond, std::initializer_list<std::function<void(LateUnscaledFixedSystem, World&)>>&& funcs) noexcept {
        lateUnscaledFixedSystems.emplace_back(std::move(cond), std::move(funcs));
    }

    void start(World& world) const noexcept {
        for (const auto& func: startSystems) {
            func(startSystem, world);
        }
    }

    void run(World& world) noexcept {
        for (const auto& mainFunc: mainSystems) {
            if (mainFunc.first == nullptr || mainFunc.first(world)) {
                for (const auto& mainRow: mainFunc.second) {
                    mainRow(mainSystem, world);
                }
            }
        }

        for (const auto& funcs: threadedSystems) {
            if (funcs.first == nullptr || funcs.first(world)) {
                if (!isUseMultithreading) {
                    for (auto& func: funcs.second) {
                        func(threadedSystem, world);
                    }
                } else {
                    threadpool.addTasks(funcs.second);
                }
            }
        }

        if (isUseMultithreading) {
            threadpool.run();
            threadpool.wait();
        }
    }

    void runLate(World& world) noexcept {
        for (const auto& lateFunc: lateSystems) {
            if (lateFunc.first == nullptr || lateFunc.first(world)) {
                for (const auto& lateRow: lateFunc.second) {
                    lateRow(lateSystem, world);
                }
            }
        }
    }

    void runThreadedFixedSetRec(World& world, const ThreadedFixedSet& set, std::vector<std::function<void(ThreadedFixedSystem, World&)>>& setTasks) noexcept {
        if (set.condition == nullptr || set.condition(world)) {
            if (!set.tasks.empty()) {
                if (!isUseMultithreading) {
                    for (auto& func: set.tasks) {
                        func(threadedFixedSystem, world);
                    }
                } else {
                    setTasks.insert(setTasks.end(), set.tasks.begin(), set.tasks.end());
                }
            }
            for (const auto& subSet: set.subSets) {
                runThreadedFixedSetRec(world, subSet, setTasks);
            }
        }
    }

    void runFixed(World& world) noexcept {
        for (const auto& [condition, systems]: mainFixedSystems) {
            if (condition == nullptr || condition(world)) {
                for (const auto& system: systems) {
                    system(mainFixedSystem, world);
                }
            }
        }

        for (const auto& subSet: threadedFixedSystems) {
            std::vector<std::function<void(ThreadedFixedSystem, World&)>> setTasks;
            runThreadedFixedSetRec(world, subSet, setTasks);
            if (isUseMultithreading && !setTasks.empty()) {
                threadpool.addFixedTasks(setTasks);
            }
        }

        if (isUseMultithreading) {
            threadpool.fixedRun();
            threadpool.fixedWait();
        }

        for (const auto& lateFunc: lateFixedSystems) {
            if (lateFunc.first == nullptr || lateFunc.first(world)) {
                for (const auto& lateRow: lateFunc.second) {
                    lateRow({}, world);
                }
            }
        }
    }

    void runUnscaledFixed(World& world) noexcept {
        for (const auto& mainFunc: mainUnscaledFixedSystems) {
            if (mainFunc.first == nullptr || mainFunc.first(world)) {
                for (const auto& mainRow: mainFunc.second) {
                    mainRow(mainUnscaledFixedSystem, world);
                }
            }
        }

        for (const auto& funcs: threadedUnscaledFixedSystems) {
            if (funcs.first == nullptr || funcs.first(world)) {
                if (!isUseMultithreading) {
                    for (auto& func: funcs.second) {
                        func(threadedUnscaledFixedSystem, world);
                    }
                } else {
                    threadpool.addUnscaledFixedTasks(funcs.second);
                }
            }
        }

        if (isUseMultithreading) {
            threadpool.unscaledFixedRun();
            threadpool.unscaledFixedWait();
        }

        for (const auto& lateFunc: lateUnscaledFixedSystems) {
            if (lateFunc.first == nullptr || lateFunc.first(world)) {
                for (const auto& lateRow: lateFunc.second) {
                    lateRow(lateUnscaledFixedSystem, world);
                }
            }
        }
    }

private:
    std::vector<std::function<void(StartSystem, World&)>> startSystems;
    std::vector<std::pair<std::function<bool(World&)>, std::vector<std::function<void(MainSystem, World&)>>>> mainSystems;
    std::vector<std::pair<std::function<bool(World&)>, std::vector<std::function<void(MainFixedSystem, World&)>>>> mainFixedSystems;
    std::vector<std::pair<std::function<bool(World&)>, std::vector<std::function<void(MainUnscaledFixedSystem, World&)>>>> mainUnscaledFixedSystems;
    std::vector<std::pair<std::function<bool(World&)>, std::vector<std::function<void(ThreadedSystem, World&)>>>> threadedSystems;
    std::vector<ThreadedFixedSet> threadedFixedSystems;
    std::vector<std::pair<std::function<bool(World&)>, std::vector<std::function<void(ThreadedUnscaledFixedSystem, World&)>>>> threadedUnscaledFixedSystems;
    std::vector<std::pair<std::function<bool(World&)>, std::vector<std::function<void(LateSystem, World&)>>>> lateSystems;
    std::vector<std::pair<std::function<bool(World&)>, std::vector<std::function<void(LateFixedSystem, World&)>>>> lateFixedSystems;
    std::vector<std::pair<std::function<bool(World&)>, std::vector<std::function<void(LateUnscaledFixedSystem, World&)>>>> lateUnscaledFixedSystems;
    ThreadPool threadpool;
    bool isUseMultithreading {true};
};

///////////////////////////////////////////////////////////////////////////////////

class Time final {
friend class ZerEngine;
public:
    Time(float newFixedTimeStep = 0.02f) noexcept:
        t2(std::chrono::high_resolution_clock::now()),
        totalTime(std::chrono::high_resolution_clock::now()),
        fixedTimeStep(newFixedTimeStep) {
    }

private:
    constexpr void setFixedTimeStep(float newFixedTimeStep) noexcept {
        fixedTimeStep = newFixedTimeStep;
    }

    void update() noexcept {
        auto t1 = std::chrono::high_resolution_clock::now();
        dt = std::chrono::duration_cast<std::chrono::duration<float>>(t1 - t2).count();
        t2 = t1;
        timeScale = newTimeScale;
        #ifdef DISPLAY_FPS
            frameCounter();
        #endif
        timeStepBuffer += dt;
        isTimeStepFrame = false;
        if (timeStepBuffer >= fixedTimeStep) {
            isTimeStepFrame = true;
            nbFixedSteps = std::floor(timeStepBuffer / fixedTimeStep);
            timeStepBuffer -= fixedTimeStep * nbFixedSteps;
        }
    }

public:
    [[nodiscard]] constexpr auto delta() const noexcept -> float {
        return dt * timeScale;
    }

    [[nodiscard]] constexpr auto unscaledDelta() const noexcept -> float {
        return dt;
    }

    [[nodiscard]] constexpr auto unscaledFixedDelta() const noexcept -> float {
        return fixedTimeStep;
    }

    [[nodiscard]] constexpr auto fixedDelta() const noexcept -> float {
        return fixedTimeStep * timeScale;
    }

    [[nodiscard]] constexpr auto isTimeStep() const noexcept -> bool {
        return isTimeStepFrame;
    }

    [[nodiscard]] constexpr auto getNbFixedSteps() const noexcept -> unsigned int {
        return nbFixedSteps;
    }

    [[nodiscard]] constexpr auto getTimeScale() const noexcept -> float {
        return newTimeScale;
    }

    constexpr void setTimeScale(const float newScale) noexcept {
        newTimeScale = newScale;
    }

private:
    #ifdef DISPLAY_FPS
        void frameCounter() noexcept {
            nbFrames++;
            timer += dt;
            if (timer >= FRAME_COOLDOWN) {
                std::println("FPS: {}", nbFrames);
                timer -= FRAME_COOLDOWN;
                nbFrames = 0;
            }
        }
    #endif

private:
    float newTimeScale = 1.0;
    float timeScale = 1.0;

private:
    double dt = 0;
    std::chrono::high_resolution_clock::time_point t2;
    std::chrono::high_resolution_clock::time_point totalTime;
    bool isTimeStepFrame = false;
    float fixedTimeStep;
    float timeStepBuffer = 0;
    unsigned int nbFixedSteps = 0;

    #ifdef DISPLAY_FPS
        float timer = 0;
        size_t nbFrames = 0;
        constexpr static float FRAME_COOLDOWN = 1.0;
    #endif
};

///////////////////////////////////////////////////////////////////////////////////

class World final {
friend class ZerEngine;
private:
    World() noexcept:
        sys(*this) {
    }

public:
    [[nodiscard("La valeur de retour d'une commande Exist doit toujours etre evalue")]] auto entity_exists(const Ent& ent) const noexcept -> bool {
        return reg.exist(ent);
    }

    template <typename T, typename... Ts> requires ((!std::is_reference_v<T> || (!std::is_reference_v<Ts> || ...)) && (!std::is_const_v<T> || (!std::is_const_v<Ts> || ...)))
    [[nodiscard("La valeur de retour d'une commande Has doit toujours etre evalue")]] auto hasThisFrame(const Ent& ent) const noexcept -> bool {
        if (!reg.exist(ent)) {
            return false;
        }
        if (reg.has(ent, {typeid(T).hash_code()})) {
            if constexpr (sizeof...(Ts) > 0) {
                return has_components<Ts...>(ent);
            }
            return true;
        }
        if (auto addEntsIt = lateUpgrade.addEnts.find(ent); addEntsIt != lateUpgrade.addEnts.end() && addEntsIt->second.contains(typeid(T).hash_code())) {
            if constexpr (sizeof...(Ts) > 0) {
                return has_components<Ts...>(ent);
            }
            return true;
        }
        if (auto addCompsIt = lateUpgrade.addComps.find(ent); addCompsIt != lateUpgrade.addComps.end() && addCompsIt->second.contains(typeid(T).hash_code())) {
            if constexpr (sizeof...(Ts) > 0) {
                return has_components<Ts...>(ent);
            }
            return true;
        }
        return false;
    }

    template <typename T, typename... Ts> requires ((!std::is_reference_v<T> || (!std::is_reference_v<Ts> || ...)) && (!std::is_const_v<T> || (!std::is_const_v<Ts> || ...)))
    [[nodiscard("La valeur de retour d'une commande Has doit toujours etre evalue")]] auto has_components(const Ent& ent) const noexcept -> bool {
        // if (!reg.exist(ent)) {
        //     return false;
        // } else if (reg.has(ent, {typeid(T).hash_code()})) {
        //     if constexpr (sizeof...(Ts) > 0) {
        //         return has<Ts...>(ent);
        //     }
        //     return true;
        // }
        // return false;
        return hasThisFrame<T, Ts...>(ent);
    }

public:
    [[nodiscard("La valeur de retour d'une commande GetTypes doit toujours etre recupere")]] auto getTypes(const Ent& ent) noexcept -> const std::vector<std::string> {
        auto types = reg.getTypes(ent);
        for (const auto& type: lateUpgrade.getTypes(ent)) {
            types.emplace_back(type);
        }
        return types;
    }

private:
    template <typename T>
    [[nodiscard]] auto internalGet(const Ent& ent) noexcept -> std::optional<std::reference_wrapper<T>> {
        if (reg.has(ent, {typeid(T).hash_code()})) {
            auto& any = reg.get(ent, typeid(T).hash_code());
            return std::any_cast<T&>(any);
        }
        return std::nullopt;
    }

    template <typename T>
    [[nodiscard]] auto internalGet(const Ent& ent) const noexcept -> std::optional<const std::reference_wrapper<T>> {
        if (reg.has(ent, {typeid(T).hash_code()})) {
            auto& any = reg.get(ent, typeid(T).hash_code());
            return std::any_cast<const T&>(any);
        }
        return std::nullopt;
    }

    template <typename T>
    [[nodiscard]] auto internalGetThisFrame(const Ent& ent) noexcept -> std::optional<std::reference_wrapper<T>> {
        if (auto addEntsIt = lateUpgrade.addEnts.find(ent); addEntsIt != lateUpgrade.addEnts.end()) {
            if (auto addEntsTypeIt = addEntsIt->second.find(typeid(T).hash_code()); addEntsTypeIt != addEntsIt->second.end()) {
                return std::any_cast<T&>(addEntsTypeIt->second);
            }
        }
        if (auto addCompsIt = lateUpgrade.addComps.find(ent); addCompsIt != lateUpgrade.addComps.end()) {
            if (auto addCompsTypeIt = addCompsIt->second.find(typeid(T).hash_code()); addCompsTypeIt != addCompsIt->second.end()) {
                return std::any_cast<T&>(addCompsTypeIt->second);
            }
        }
        if (reg.has(ent, {typeid(T).hash_code()})) {
            auto& any = reg.get(ent, typeid(T).hash_code());
            return std::any_cast<T&>(any);
        }
        return std::nullopt;
    }

    template <typename T>
    [[nodiscard]] auto internalGetThisFrame(const Ent& ent) const noexcept -> std::optional<const std::reference_wrapper<T>> {
        if (auto addEntsIt = lateUpgrade.addEnts.find(ent); addEntsIt != lateUpgrade.addEnts.end()) {
            if (auto addEntsTypeIt = addEntsIt->second.find(typeid(T).hash_code()); addEntsTypeIt != addEntsIt->second.end()) {
                return std::any_cast<const T&>(addEntsTypeIt->second);
            }
        }
        if (auto addCompsIt = lateUpgrade.addComps.find(ent); addCompsIt != lateUpgrade.addComps.end()) {
            if (auto addCompsTypeIt = addCompsIt->second.find(typeid(T).hash_code()); addCompsTypeIt != addCompsIt->second.end()) {
                return std::any_cast<const T&>(addCompsTypeIt->second);
            }
        }
        if (reg.has(ent, {typeid(T).hash_code()})) {
            auto& any = reg.get(ent, typeid(T).hash_code());
            return std::any_cast<const T&>(any);
        }
        return std::nullopt;
    }

public:
    template <typename T, typename... Ts> requires (IsNotEmptyConcept<T> && IsFinalConcept<T> && IsNotSameConcept<T, Ts...> && !std::is_reference_v<T>)
    [[nodiscard("La valeur de retour d'une commande Get doit toujours etre recupere")]] auto getThisFrame(const Ent& ent) noexcept -> std::optional<std::tuple<T&, Ts&...>> {
        if (auto opt = internalGetThisFrame<T>(ent)) {
            if constexpr (sizeof...(Ts) > 0) {
                if (auto othOpt = getThisFrame<Ts...>(ent)) {
                    return std::tuple_cat(std::forward_as_tuple(opt.value().get()), othOpt.value());
                }
                return std::nullopt;
            } else {
                return std::forward_as_tuple(opt.value().get());
            }
        }
        return std::nullopt;
    }

    template <typename T, typename... Ts> requires (IsNotEmptyConcept<T> && IsFinalConcept<T> && IsNotSameConcept<T, Ts...> && !std::is_reference_v<T>)
    [[nodiscard("La valeur de retour d'une commande Get doit toujours etre recupere")]] auto get_components(const Ent& ent) noexcept -> std::optional<std::tuple<T&, Ts&...>> {
        // if (auto opt = internalGet<T>(ent)) {
        //     if constexpr (sizeof...(Ts) > 0) {
        //         if (auto othOpt = get<Ts...>(ent)) {
        //             return std::tuple_cat(std::forward_as_tuple(opt.value()), othOpt.value());
        //         } else {
        //             return std::nullopt;
        //         }
        //     } else {
        //         return std::forward_as_tuple(opt.value());
        //     }
        // }
        // return std::nullopt;
        return getThisFrame<T, Ts...>(ent);
    }

    [[nodiscard("La valeur de retour d'une commande HasParent doit toujours etre evaluer")]] auto has_parent(const Ent& childEnt) const noexcept -> bool {
        return reg.hasParent(childEnt);
    }

    [[nodiscard("La valeur de retour d'une commande GetParent doit toujours etre recupere")]] auto get_parent(const Ent& childEnt) const noexcept -> std::optional<Ent> {
        return reg.getParent(childEnt);
    }

    [[nodiscard("La valeur de retour d'une commande HasChildren doit toujours etre evaluer")]] auto has_children(const Ent& parentEnt) const noexcept -> bool {
        return reg.hasChildren(parentEnt);
    }

    [[nodiscard("La valeur de retour d'une commande GetChildren doit toujours etre recupere")]] auto get_children(const Ent& parentEnt) const noexcept -> std::optional<std::reference_wrapper<const std::unordered_set<Ent>>> {
        return reg.getChildren(parentEnt);
    }

    constexpr auto append_children(const Ent& parentEnt, const std::vector<Ent>& childrenEnt) noexcept -> Ent {
        lateUpgrade.appendChildren(parentEnt, childrenEnt);
        return parentEnt;
    }

    template <typename... Ts> requires ((sizeof...(Ts) > 0) && (!std::is_reference_v<Ts> || ...))
    [[nodiscard("La valeur de retour d'une commande Resource doit toujours etre recupere")]] auto resource(this auto& self) noexcept -> std::tuple<Ts&...> {
        return std::forward_as_tuple(std::any_cast<Ts&>(self.res.get(typeid(Ts).hash_code()))...);
    }

    [[nodiscard]] auto getDestroyedEnts() const noexcept -> const std::unordered_set<Ent>& {
        return lateUpgrade.delEnts;
    }

    [[nodiscard]] auto getAddedEnts() const noexcept -> const std::unordered_map<Ent, std::unordered_map<Type, std::any>>& {
        return lateUpgrade.addEnts;
    }

    [[nodiscard]] auto getAddedComps() const noexcept -> const std::unordered_map<Ent, std::unordered_map<Type, std::any>>& {
        return lateUpgrade.addComps;
    }

    [[nodiscard]] auto getDestroyedComps() const noexcept -> const std::unordered_map<Ent, std::unordered_set<Type>>& {
        return lateUpgrade.delComps;
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        !(std::is_reference_v<Comps> || ...) &&
        !((std::is_const_v<Filters> || std::is_reference_v<Filters>) || ...) &&
        !((std::is_const_v<Excludes> || std::is_reference_v<Excludes>) || ...)
    )
    [[nodiscard]] auto view(With<Filters...> = {}, Without<Excludes...> = {}) noexcept -> const View<Comps...> {
        return reg.view<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(IsInactive).hash_code(), typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        !(std::is_reference_v<Comps> || ...) &&
        !((std::is_const_v<Filters> || std::is_reference_v<Filters>) || ...) &&
        !((std::is_const_v<Excludes> || std::is_reference_v<Excludes>) || ...)
    )
    [[nodiscard]] auto view(Without<Excludes...>, With<Filters...> = {}) noexcept -> const View<Comps...> {
        return reg.view<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(IsInactive).hash_code(), typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        !(std::is_reference_v<Comps> || ...) &&
        !((std::is_const_v<Filters> || std::is_reference_v<Filters>) || ...) &&
        !((std::is_const_v<Excludes> || std::is_reference_v<Excludes>) || ...)
    )
    [[nodiscard]] auto view(With<Filters...>, Without<Excludes...>, WithInactive) noexcept -> const View<Comps...> {
        return reg.view<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        !(std::is_reference_v<Comps> || ...) &&
        !((std::is_const_v<Filters> || std::is_reference_v<Filters>) || ...) &&
        !((std::is_const_v<Excludes> || std::is_reference_v<Excludes>) || ...)
    )
    [[nodiscard]] auto view(Without<Excludes...>, With<Filters...>, WithInactive) noexcept -> const View<Comps...> {
        return reg.view<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        !(std::is_reference_v<Comps> || ...) &&
        !((std::is_const_v<Filters> || std::is_reference_v<Filters>) || ...) &&
        !((std::is_const_v<Excludes> || std::is_reference_v<Excludes>) || ...)
    )
    [[nodiscard]] auto view(With<Filters...>, WithInactive, Without<Excludes...> = {}) noexcept -> const View<Comps...> {
        return reg.view<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        !(std::is_reference_v<Comps> || ...) &&
        !((std::is_const_v<Filters> || std::is_reference_v<Filters>) || ...) &&
        !((std::is_const_v<Excludes> || std::is_reference_v<Excludes>) || ...)
    )
    [[nodiscard]] auto view(Without<Excludes...>, WithInactive, With<Filters...> = {}) noexcept -> const View<Comps...> {
        return reg.view<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        !(std::is_reference_v<Comps> || ...) &&
        !((std::is_const_v<Filters> || std::is_reference_v<Filters>) || ...) &&
        !((std::is_const_v<Excludes> || std::is_reference_v<Excludes>) || ...)
    )
    [[nodiscard]] auto view(WithInactive, With<Filters...> = {}, Without<Excludes...> = {}) noexcept -> const View<Comps...> {
        return reg.view<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        !(std::is_reference_v<Comps> || ...) &&
        !((std::is_const_v<Filters> || std::is_reference_v<Filters>) || ...) &&
        !((std::is_const_v<Excludes> || std::is_reference_v<Excludes>) || ...)
    )
    [[nodiscard]] auto view(WithInactive, Without<Excludes...>, With<Filters...> = {}) noexcept -> const View<Comps...> {
        return reg.view<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(Excludes).hash_code()...});
    }

    template <typename... Comps> requires ((std::copy_constructible<Comps> && ...) && (IsFinalConcept<Comps> && ...) && IsNotSameConcept<Comps...>)
    auto create_entity(const Comps&... comps) noexcept -> const Ent {
        return lateUpgrade.newEnt(
            reg.getEntToken(),
            {{typeid(Comps).hash_code(), std::make_any<Comps>(comps)}...}
        );
    }

    template <typename Comp, typename... Comps> requires ((std::copy_constructible<Comp>) && (IsFinalConcept<Comp>) && IsNotSameConcept<Comps...>)
    auto add_components(const Ent& ent, const Comp& comp, const Comps&... comps) noexcept -> std::optional<std::tuple<Comp&, Comps&...>> {
        if (reg.exist(ent)) {
            lateUpgrade.add(
                reg,
                ent,
                std::initializer_list<std::pair<const Type, std::any>>{
                    {typeid(Comp).hash_code(), std::make_any<Comp>(comp)},
                    {typeid(Comps).hash_code(), std::make_any<Comps>(comps)}...
                }
            );
        } else {
            std::println("World::add_component(): Impossible d'ajouter sur une entitée qui n'existe pas [type: {}]", typeid(Comp).name());
            (std::println("World::add_component(): Impossible d'ajouter sur une entitée qui n'existe pas [type: {}]", typeid(Comps).name()), ...);
            return std::nullopt;
        }

        return std::forward_as_tuple(internalGetThisFrame<Comp>(ent).value(), internalGetThisFrame<Comps>(ent).value()...);
    }

    template <typename T, typename... Ts>
    void remove_components(const Ent& ent) noexcept {
        if (reg.has(ent, {typeid(T).hash_code()})) {
            lateUpgrade.remove(ent, typeid(T).hash_code());
        } else {
            std::println("World::remove_component(): Impossible de supprimer un composant qui n'existe pas - {}", typeid(T).name());
        }

        if constexpr (sizeof...(Ts) > 0) {
            remove_components<Ts...>(ent);
        }
    }

    void delete_entity(const Ent& ent) noexcept {
        if (reg.exist(ent)) {
            lateUpgrade.destroy(ent);
        } else {
            std::println("World::delete_entity(): Impossible de supprimer une entitée qui n'existe pas");
        }
    }

    void setActive(const Ent& ent) noexcept {
        lateUpgrade.setActive(ent);
    }

    void setInactive(const Ent& ent) noexcept {
        lateUpgrade.setInactive(ent);
    }

    [[nodiscard]] constexpr auto getTotalEntities() const noexcept -> std::size_t {
        return reg.entArch.size();
    }

    [[nodiscard]] constexpr auto getTotalArchetypes() const noexcept -> std::size_t {
        return reg.archs.size();
    }

    void addDontDestroyOnLoad(const Ent& ent) noexcept {
        lateUpgrade.addDontDestroyOnLoad(ent);
    }

    void loadScene(std::function<void(SceneSystem, World&)>&& newScene) noexcept {
        lateUpgrade.loadScene(std::move(newScene));
    }

    void stop_run(bool val = true) noexcept {
        isRunning = !val;
    }

    void upgrade() noexcept {
        lateUpgrade.upgrade(*this, reg);
    }

private:
    TypeMap res;
    Registry reg;
    LateUpgrade lateUpgrade;
    Sys sys;
    bool isRunning;
};

///////////////////////////////////////////////////////////////////////////////////

class ZerEngine final {
public:
    ZerEngine() noexcept {
        world.res.emplace(typeid(Time).hash_code(), std::make_any<Time>(0.02f));
    }

    [[nodiscard]] constexpr auto use_multithreading(bool newVal) noexcept -> ZerEngine& {
        world.sys.useMultithreading(newVal);
        return *this;
    }

    [[nodiscard]] constexpr auto set_fixed_time_step(float newFixedTimeStep) noexcept -> ZerEngine& {
        auto [time] = world.resource<Time>();
        time.setFixedTimeStep(newFixedTimeStep);
        return *this;
    }

    template <typename T, typename... Args> requires ((std::copy_constructible<T>) && (IsFinalConcept<T>))
    [[nodiscard]] auto add_resource(Args&&... args) noexcept -> ZerEngine& {
        world.res.emplace(typeid(T).hash_code(), std::make_any<T>(std::forward<Args>(args)...));
        return *this;
    }

    template <typename T, typename... Args>
    [[nodiscard]] auto add_plugin(std::function<void(ZerEngine&)>&& pluginFunc) noexcept -> ZerEngine& {
        pluginFunc(*this);
        return *this;
    }

    [[nodiscard]] auto add_systems(StartSystem, std::function<void(StartSystem, World&)>&& func) noexcept -> ZerEngine& {
        world.sys.addStartSys(std::move(func));
        return *this;
    }

    [[nodiscard]] auto add_systems(MainSystem, std::initializer_list<std::function<void(MainSystem, World&)>>&& funcs) noexcept -> ZerEngine& {
        world.sys.addMainCondSys(nullptr, std::move(funcs));
        return *this;
    }

    [[nodiscard]] auto add_systems(MainSystem, std::function<bool(World&)>&& cond, std::initializer_list<std::function<void(MainSystem, World&)>>&& funcs) noexcept -> ZerEngine& {
        world.sys.addMainCondSys(std::move(cond), std::move(funcs));
        return *this;
    }

    [[nodiscard]] auto add_systems(MainFixedSystem, std::initializer_list<std::function<void(MainFixedSystem, World&)>>&& funcs) noexcept -> ZerEngine& {
        world.sys.addMainFixedCondSys(nullptr, std::move(funcs));
        return *this;
    }

    [[nodiscard]] auto add_systems(MainFixedSystem, std::function<bool(World&)>&& cond, std::initializer_list<std::function<void(MainFixedSystem, World&)>>&& funcs) noexcept -> ZerEngine& {
        world.sys.addMainFixedCondSys(std::move(cond), std::move(funcs));
        return *this;
    }

    [[nodiscard]] auto add_systems(MainUnscaledFixedSystem, std::initializer_list<std::function<void(MainUnscaledFixedSystem, World&)>>&& funcs) noexcept -> ZerEngine& {
        world.sys.addMainUnscaledFixedCondSys(nullptr, std::move(funcs));
        return *this;
    }

    [[nodiscard]] auto add_systems(MainUnscaledFixedSystem, std::function<bool(World&)>&& cond, std::initializer_list<std::function<void(MainUnscaledFixedSystem, World&)>>&& funcs) noexcept -> ZerEngine& {
        world.sys.addMainUnscaledFixedCondSys(std::move(cond), std::move(funcs));
        return *this;
    }

    [[nodiscard]] auto add_systems(ThreadedSystem, std::initializer_list<std::function<void(ThreadedSystem, World&)>>&& funcs) noexcept -> ZerEngine& {
        world.sys.addThreadedCondSys(nullptr, std::move(funcs));
        return *this;
    }

    [[nodiscard]] auto add_systems(ThreadedSystem, std::function<bool(World&)>&& cond, std::initializer_list<std::function<void(ThreadedSystem, World&)>>&& funcs) noexcept -> ZerEngine& {
        world.sys.addThreadedCondSys(std::move(cond), std::move(funcs));
        return *this;
    }

    [[nodiscard]] auto add_systems(const ThreadedFixedSet& newSet) noexcept -> ZerEngine& {
        world.sys.addThreadedFixedCondSys(newSet);
        return *this;
    }

    [[nodiscard]] auto add_systems(ThreadedUnscaledFixedSystem, std::initializer_list<std::function<void(ThreadedUnscaledFixedSystem, World&)>>&& funcs) noexcept -> ZerEngine& {
        world.sys.addThreadedUnscaledFixedCondSys(nullptr, std::move(funcs));
        return *this;
    }

    [[nodiscard]] auto add_systems(ThreadedUnscaledFixedSystem, std::function<bool(World&)>&& cond, std::initializer_list<std::function<void(ThreadedUnscaledFixedSystem, World&)>>&& funcs) noexcept -> ZerEngine& {
        world.sys.addThreadedUnscaledFixedCondSys(std::move(cond), std::move(funcs));
        return *this;
    }

    [[nodiscard]] auto add_systems(LateSystem, std::initializer_list<std::function<void(LateSystem, World&)>>&& funcs) noexcept -> ZerEngine& {
        world.sys.addLateCondSys(nullptr, std::move(funcs));
        return *this;
    }

    [[nodiscard]] auto add_systems(LateSystem, std::function<bool(World&)>&& cond, std::initializer_list<std::function<void(LateSystem, World&)>>&& funcs) noexcept -> ZerEngine& {
        world.sys.addLateCondSys(std::move(cond), std::move(funcs));
        return *this;
    }

    [[nodiscard]] auto add_systems(LateFixedSystem, std::initializer_list<std::function<void(LateFixedSystem, World&)>>&& funcs) noexcept -> ZerEngine& {
        world.sys.addLateFixedCondSys(nullptr, std::move(funcs));
        return *this;
    }

    [[nodiscard]] auto add_systems(LateFixedSystem, std::function<bool(World&)>&& cond, std::initializer_list<std::function<void(LateFixedSystem, World&)>>&& funcs) noexcept -> ZerEngine& {
        world.sys.addLateFixedCondSys(std::move(cond), std::move(funcs));
        return *this;
    }

    [[nodiscard]] auto add_systems(LateUnscaledFixedSystem, std::initializer_list<std::function<void(LateUnscaledFixedSystem, World&)>>&& funcs) noexcept -> ZerEngine& {
        world.sys.addLateUnscaledFixedCondSys(nullptr, std::move(funcs));
        return *this;
    }

    [[nodiscard]] auto add_systems(LateUnscaledFixedSystem, std::function<bool(World&)>&& cond, std::initializer_list<std::function<void(LateUnscaledFixedSystem, World&)>>&& funcs) noexcept -> ZerEngine& {
        world.sys.addLateUnscaledFixedCondSys(std::move(cond), std::move(funcs));
        return *this;
    }

    void run() noexcept {
        world.isRunning = true;
        world.sys.start(world);
        world.upgrade();
        while (world.isRunning) {
            auto [time] = world.resource<Time>();
            time.update();

            world.sys.run(world);

            if (time.isTimeStep()) {
                if (time.timeScale != 0) {
                    for (unsigned int i = 0; i < time.getNbFixedSteps(); i++) {
                        world.upgrade();
                        world.sys.runFixed(world);
                    }
                }
                for (unsigned int i = 0; i < time.getNbFixedSteps(); i++) {
                    world.upgrade();
                    world.sys.runUnscaledFixed(world);
                }
            }

            world.upgrade();
            world.sys.runLate(world);
            world.upgrade();
        }
    }

private:
    World world;
};

///////////////////////////////////////////////////////////////////////////////////