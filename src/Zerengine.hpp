#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <iostream>
#include <any>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <semaphore>
#include <functional>
#include <concepts>
#include <optional>

using Ent = std::size_t;
using Type = std::size_t;

template <typename... Filters>
struct With final {};
template <typename... Filters>
constinit With<Filters...> with;

template <typename... Excludes>
struct Without final {};
template <typename... Excludes>
constinit Without<Excludes...> without;

struct WithInactive final {};
constinit WithInactive withInactive;

class IsInactive final {};
class DontDestroyOnLoad final {};

class StartSystem final {};
class MainSystem final {};
class MainFixedSystem final {};
class ThreadedSystem final {};
class ThreadedFixedSystem final {};
class LateSystem final {};
class LateFixedSystem final {};

///////////////////////////////////////////////////////////////////////////////////

// class Comp {
// public:
//     constexpr Comp() noexcept:
//         manager(nullptr) {
//     }

//     Comp(const Comp& othComp) {
//         if (!othComp.has_value()) {
//             manager = nullptr;
//         } else {
//             Arg arg;
//             arg.comp = this;
//             othComp.manager(OP_CLONE, &othComp, &arg);
//         }
//     }

//     Comp(Comp&& othComp) {
//         if (!othComp.has_value()) {
//             manager = nullptr;
//         } else {
//             Arg arg;
//             arg.comp = this;
//             othComp.manager(OP_TRANSFERT, &othComp, &arg);
//         }
//     }

//     template <typename T> requires (std::copy_constructible<T> && !std::same_as<T, Comp>)
//     Comp(T&& newComp):
//         data(new T(std::forward<T>(newComp))),
//         manager(managerFunc<T>) {
//     }

//     template <typename T, typename... Args>
//     Comp(std::in_place_type_t<T>, Args&&... args):
//         data(new T(std::forward<Args>(args)...)),
//         manager(managerFunc<T>) {
//     }

//     ~Comp() {
//         reset();
//     }

//     Comp& operator =(const Comp& othComp) {
//         *this = Comp(othComp);
//         return *this;
//     }

//     Comp& operator =(Comp&& othComp) {
//         if (!othComp.has_value()) {
//             reset();
//         } else if (this != &othComp) {
//             reset();
//             Arg arg;
//             arg.comp = this;
//             othComp.manager(OP_TRANSFERT, &othComp, &arg);
//         }
//         return *this;
//     }

//     template <typename T>
//     Comp& operator =(T&& othComp) {
//         *this = Comp(std::forward<T>(othComp));
//         return *this;
//     }

// public:
//     void reset() noexcept {
//         if (has_value()) {
//             manager(OP_DESTROY, this, nullptr);
//             manager = nullptr;
//         }
//     }

//     const std::type_info& type() const noexcept {
//         if (!has_value()) {
//             return typeid(void);
//         }
//         Arg arg;
//         manager(OP_GET_TYPE, this, &arg);
//         return *arg.typeInfo;
//     }

//     [[nodiscard]] bool has_value() const noexcept {
//         return manager != nullptr;
//     }

// public:
//     template <typename T>
//     friend T* compCast(const Comp* comp) noexcept {
//         if (comp && comp->type() == typeid(T)) {
//             return static_cast<T*>(comp->data);
//         }
//         printf("CompCast Impossible: Comp<%s> voulu en %s", comp->type().name(), typeid(T).name());
//         return nullptr;
//     }

// private:
//     enum Op {
//         OP_ACCESS, OP_GET_TYPE, OP_CLONE, OP_DESTROY, OP_TRANSFERT
//     };

//     union Arg {
//         void* obj;
//         const std::type_info* typeInfo;
//         Comp* comp;
//     };

// private:
//     template <typename T>
//     static void managerFunc(Op op, const Comp* comp, Arg* arg) {
//         const T* ptr = static_cast<const T*>(comp->data);
//         switch (op) {
//             case OP_ACCESS:
//                 arg->obj = const_cast<T*>(ptr);
//                 break;
//             case OP_GET_TYPE:
//                 arg->typeInfo = &typeid(T);
//                 break;
//             case OP_CLONE:
//                 arg->comp->data = new T(*ptr);
//                 arg->comp->manager = comp->manager;
//                 break;
//             case OP_DESTROY:
//                 delete ptr;
//                 break;
//             case OP_TRANSFERT:
//                 arg->comp->data = comp->data;
//                 arg->comp->manager = comp->manager;
//                 const_cast<Comp*>(comp)->manager = nullptr;
//                 break;
//         }
//     }

// private:
//     void* data;
//     void(*manager)(Op, const Comp*, Arg*);
// };

///////////////////////////////////////////////////////////////////////////////////

class CompPool final {
friend class Archetype;
friend class LiteArchetype;
friend class LateUpgrade;
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
        return std::move(self).comps.at(ent);
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

///////////////////////////////////////////////////////////////////////////////////

struct ArchetypeCreateWith final {};
constinit ArchetypeCreateWith archetypeCreateWith;

struct ArchetypeCreateWithout final {};
constinit ArchetypeCreateWithout archetypeCreateWithout;

class Archetype final {
friend class Registry;
friend class LiteArchetype;
friend class LateUpgrade;
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
        pools(oldArch.pools.size() + 1) {
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
        pools(oldArch.pools.size() - 1) {
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
    [[nodiscard]] bool empty() const noexcept {
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

    [[nodiscard]] std::size_t size() const noexcept {
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

        [[nodiscard]] value_type operator *() const noexcept {
            return (*archsIt)->getTupleWithEnt<Ts...>((*entsIt));
        }

        [[nodiscard]] ViewIterator& operator ++() noexcept {
            entsIt++;
            if (entsIt == (*archsIt)->ents.end()) {
                archsIt++;
                if (archsIt != archs.end()) {
                    entsIt = (*archsIt)->ents.begin();
                }
            }
            return *this;
        }

        [[nodiscard]] friend constexpr bool operator !=(const ViewIterator& a, const ViewIterator& b) noexcept {
            return a.archsIt != b.archsIt;
        }

    private:
        std::unordered_set<Archetype*>::const_iterator archsIt;
        std::unordered_set<Ent>::iterator entsIt;
        const std::unordered_set<Archetype*>& archs;
    };

public:
    [[nodiscard]] constexpr ViewIterator begin() const noexcept {
        return {archs, archs.begin()};
    }

    [[nodiscard]] constexpr ViewIterator end() const noexcept {
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
        emptyArch(new Archetype()) {
    }

private:
    ~Registry() noexcept {
        for (auto* arch: archs) {
            delete arch;
        }
        delete emptyArch;
    }

    [[nodiscard]] Ent getEntToken() noexcept {
        Ent ent = lastEnt++;

        if (!entTokens.empty()) {
            lastEnt--;
            ent = entTokens.back();
            entTokens.pop_back();
        }

        entArch.emplace(ent, emptyArch);

        return ent;
    }

    void newEnt(const Ent ent, const std::unordered_map<Type, std::any>& anyes) noexcept {
        auto entArchIt = entArch.find(ent);
        if (entArchIt->second->size() > 0) {
            printf("ZerEngine: Impossible d'inserer une entité deja existante - [%zu]\n", ent);
            return;
        }

        std::unordered_set<Archetype*> compatiblesArchs(archs);
        for (const auto& pairAny: anyes) {
            filterArchsByType(pairAny.first, compatiblesArchs);
        }
        for (auto* arch: compatiblesArchs) {
            if (arch->isTotalyCompatibleLate(anyes)) {
                arch->newEnt(ent, anyes);
                entArchIt->second = arch;
                for (const auto& pair: arch->pools) {
                    emplaceArchByType(pair.first, arch);
                }
                return;
            }
        }

        auto* arch = new Archetype(ent, anyes);
        archs.emplace(arch);
        entArchIt->second = arch;
        for (const auto& pair: arch->pools) {
            emplaceArchByType(pair.first, arch);
        }
    }

    void add(const Ent ent, const std::any& any) noexcept {
        auto entArchIt = entArch.find(ent);
        if (entArchIt == entArch.end()) {
            printf("ZerEngine: Impossible d'ajouter un composant sur une entité inexistante - [%zu]\n", ent);
            return;
        }

        if (entArchIt->second->contains(any.type().hash_code())) {
            printf("ZerEngine: Impossible d'ajouter 2 composants identiques  - [%zu] - %s\n", ent, any.type().name());
            return;
        }

        auto* oldArch = entArchIt->second;

        std::unordered_set<Archetype*> compatiblesArchs(archs);
        for (const auto& pairPools: oldArch->pools) {
            filterArchsByType(pairPools.first, compatiblesArchs);
        }
        filterArchsByType(any.type().hash_code(), compatiblesArchs);
        for (auto* arch: compatiblesArchs) {
            if (arch->isTotalyCompatibleLate(*oldArch, any.type().hash_code())) {
                arch->add(ent, *oldArch, any);
                entArchIt->second = arch;
                emplaceArchByType(any.type().hash_code(), arch);
                removeOldArchIfEmpty(oldArch);
                return;
            }
        }

        auto* arch = new Archetype(archetypeCreateWith, *oldArch, ent, any);
        archs.emplace(arch);
        entArchIt->second = arch;
        for (const auto& pair: arch->pools) {
            emplaceArchByType(pair.first, arch);
        }
        removeOldArchIfEmpty(oldArch);
    }

    void del(const Ent ent, const Type type) noexcept {
        auto entArchIt = entArch.find(ent);
        if (entArchIt == entArch.end()) {
            printf("ZerEngine: Impossible de supprimer un composant sur une entite inexistante - [%zu]\n", ent);
            return;
        }

        if (!entArchIt->second->contains(type)) {
            printf("ZerEngine: Impossible de supprimer un composant qui n'existe pas - [%zu]\n", ent);
            return;
        }

        auto* oldArch = entArchIt->second;

        std::unordered_set<Archetype*> compatiblesArchs(archs);
        for (const auto& pairPools: oldArch->pools) {
            if (pairPools.first != type) {
                filterArchsByType(pairPools.first, compatiblesArchs);
            }
        }
        for (auto* arch: compatiblesArchs) {
            if (arch->isTotalyCompatibleWithoutLate(*oldArch, type)) {
                arch->del(ent, *oldArch, type);
                entArchIt->second = arch;
                removeOldArchIfEmpty(oldArch);
                return;
            }
        }

        auto* arch = new Archetype(archetypeCreateWithout, *oldArch, ent, type);
        archs.emplace(arch);
        entArchIt->second = arch;
        for (const auto& pair: arch->pools) {
            emplaceArchByType(pair.first, arch);
        }
        removeOldArchIfEmpty(oldArch);
    }

    [[nodiscard]] constexpr bool exist(const Ent ent) const noexcept {
        return entArch.contains(ent);
    }

    [[nodiscard]] bool has(const Ent ent, const std::initializer_list<Type>& types) const noexcept {
        auto entArchIt = entArch.find(ent);
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

    [[nodiscard]] auto&& get(this auto&& self, const Ent ent, const Type type) noexcept {
        return self.entArch.at(ent)->get(ent, type);
    }

    const std::vector<std::string> getTypes(const Ent ent) const noexcept {
        return entArch.at(ent)->getTypes(ent);
    }

    void destroy(const Ent ent) noexcept {
        auto entArchIt = entArch.find(ent);
        if (entArchIt == entArch.end()) {
            printf("ZerEngine: Impossible de detruire une entitée qui n'existe pas\n");
            return;
        }
 
        auto* arch = entArchIt->second;
        arch->destroy(ent);
        removeOldArchIfEmpty(arch);
        entArch.erase(entArchIt);
        entTokens.push_back(ent);
        detachChildren(ent);
        removeParent(ent);
    }

    void clean() noexcept {
        for (auto* arch: archs) {
            delete arch;
        }
        delete emptyArch;

        emptyArch = new Archetype();
        lastEnt = 1;
        entTokens.clear();
        archs.clear();
        entArch.clear();
        archsByType.clear();
        parentChildrens.clear();
        childrenParent.clear();
    }

private:
    void appendChildren(const Ent parentEnt, const std::unordered_set<Ent>& childrenEnt) noexcept {
        auto parentIt = parentChildrens.find(parentEnt);
        if (parentIt == parentChildrens.end()) {
            parentIt = parentChildrens.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(parentEnt),
                std::forward_as_tuple()
            ).first;
        }
        for (const auto childEnt: childrenEnt) {
            if (childrenParent.contains(childEnt)) {
                printf("Children: Tu ne peux pas avoir deux parents Billy[%zu]\n", childEnt);
            } else if (parentEnt == childEnt) {
                printf("Children: Impossible d'etre son propre pere\n");  
            } else {
                childrenParent.emplace(childEnt, parentEnt);
                parentIt->second.emplace(childEnt);
            }
        }
        if (parentIt->second.empty()) {
            parentChildrens.erase(parentIt);
        }
    }

    void appendChildren(const Ent parentEnt, const std::vector<Ent>& childrenEnt) noexcept {
        auto parentIt = parentChildrens.find(parentEnt);
        if (parentIt == parentChildrens.end()) {
            parentIt = parentChildrens.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(parentEnt),
                std::forward_as_tuple()
            ).first;
        }
        for (const auto childEnt: childrenEnt) {
            if (childrenParent.contains(childEnt)) {
                printf("Children: Tu ne peux pas avoir deux parents Billy[%zu]\n", childEnt);
            } else if (parentEnt == childEnt) {
                printf("Children: Impossible d'etre son propre pere\n");  
            } else {
                childrenParent.emplace(childEnt, parentEnt);
                parentIt->second.emplace(childEnt);
            }
        }
        if (parentIt->second.empty()) {
            parentChildrens.erase(parentIt);
        }
    }

    void detachChildren(const Ent parentEnt) noexcept {
        if (auto parentIt = parentChildrens.find(parentEnt); parentIt != parentChildrens.end()) {
            for (const auto childEnt: parentIt->second) {
                if (auto childrenIt = childrenParent.find(childEnt); childrenIt != childrenParent.end()) {
                    childrenParent.erase(childrenIt);
                }
            }
            parentChildrens.erase(parentIt);
        }
    }

    void removeParent(const Ent childEnt) noexcept {
        if (auto childrenIt = childrenParent.find(childEnt); childrenIt != childrenParent.end()) {
            if (auto parentIt = parentChildrens.find(childrenIt->second); parentIt != parentChildrens.end()) {
                parentIt->second.erase(childEnt);
                if (parentIt->second.empty()) {
                    parentChildrens.erase(parentIt);
                }
            }
            childrenParent.erase(childrenIt);
        }
    }

    [[nodiscard]] bool hasChildren(const Ent parentEnt) const noexcept {
        return parentChildrens.contains(parentEnt);
    }

    [[nodiscard]] std::optional<std::reference_wrapper<const std::unordered_set<Ent>>> getChildren(const Ent parentEnt) const noexcept {
        if (auto parentIt = parentChildrens.find(parentEnt); parentIt != parentChildrens.end()) {
            return std::make_optional<std::reference_wrapper<const std::unordered_set<Ent>>>(std::reference_wrapper<const std::unordered_set<Ent>>(parentIt->second));
        }
        return std::nullopt;
    }

    [[nodiscard]] bool hasParent(const Ent childEnt) const noexcept {
        return childrenParent.contains(childEnt);
    }

    [[nodiscard]] std::optional<Ent> getParent(const Ent childEnt) const noexcept {
        if (auto childIt = childrenParent.find(childEnt); childIt != childrenParent.end()) {
            return childIt->second;
        }
        return std::nullopt;
    }

private:
    template <typename... Comps>
    [[nodiscard]] const View<Comps...> view(const std::initializer_list<Type>& compFilterTypes, const std::initializer_list<Type>& excludeTypes) const noexcept {
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
    void viewAddComp(std::unordered_set<Archetype*>& internalArchs, const std::initializer_list<Type>& compTypes) const noexcept {
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

    void viewWithoutComp(std::unordered_set<Archetype*>& internalArchs, const std::initializer_list<Type>& compTypes) const noexcept {
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

    void filterArchsByType(const Type type, std::unordered_set<Archetype*>& compatibleArchs) noexcept {
        std::unordered_set<Archetype*> newArchs;
        if (auto archsByTypeIt = archsByType.find(type); archsByTypeIt != archsByType.end()) {
            for (auto* arch: archsByTypeIt->second) {
                if (compatibleArchs.contains(arch)) {
                    newArchs.emplace(arch);
                }
            }
        }
        compatibleArchs = newArchs;
    }

private:
    void emplaceArchByType(const Type type, Archetype* arch) noexcept {
        if (auto archsByTypeIt = archsByType.find(type); archsByTypeIt != archsByType.end()) {
            archsByTypeIt->second.emplace(arch);
        } else {
            archsByType.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(type),
                std::forward_as_tuple(std::initializer_list<Archetype*>{arch})
            );
        }
    }

    void removeOldArchIfEmpty(Archetype* oldArch) noexcept {
        if (oldArch->size() <= 0 && oldArch != emptyArch) {
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
    Archetype* emptyArch;
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
    Ent newEnt(const Ent ent, const std::initializer_list<std::pair<const Type, std::any>>& newList) noexcept {
        std::unique_lock<std::mutex> lock(mtx);
        addEnts.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(ent),
            std::forward_as_tuple(newList)
        );
        return ent;
    }

    void add(const Ent ent, const std::initializer_list<std::pair<const Type, std::any>>& newList) noexcept {
        std::unique_lock<std::mutex> lock(mtx);
        if (auto addEntsIt = addEnts.find(ent); addEntsIt != addEnts.end()) {
            for (const auto& pair: newList) {
                if (!addEntsIt->second.contains(pair.first)) {
                    addEntsIt->second.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(pair.first),
                        std::forward_as_tuple(pair.second)
                    );
                } else {
                    printf("No Add Sur Ent: Le Composant %s existe deja\n", pair.second.type().name());
                }
            }
        } else {
            auto addCompsIt = addComps.find(ent);
            if (addCompsIt == addComps.end()) {
                addComps.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(ent),
                    std::forward_as_tuple(newList)
                );
            } else {
                for (const auto& pair: newList) {
                    if (!addCompsIt->second.contains(pair.first)) {
                        addCompsIt->second.emplace(
                            std::piecewise_construct,
                            std::forward_as_tuple(pair.first),
                            std::forward_as_tuple(pair.second)
                        );
                    } else {
                        printf("No Add Sur Comp: Le Composant %s existe deja\n", pair.second.type().name());
                    }
                }
            }
        }
    }

    void del(const Ent ent, const Type type) noexcept {
        std::unique_lock<std::mutex> lock(mtx);
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

    void destroyChildRec(Registry& reg, const Ent parentEnt) noexcept {
        addEnts.erase(parentEnt);
        addComps.erase(parentEnt);
        delComps.erase(parentEnt);
        delEnts.emplace(parentEnt);
        if (addComps.contains(parentEnt)) {
            addComps.erase(parentEnt);
        }
        if (auto childrenOpt = reg.getChildren(parentEnt)) {
            for (const auto childEnt: childrenOpt.value().get()) {
                destroyChildRec(reg, childEnt);
            }
        }
    }

    void destroy(Registry& reg, const Ent ent) noexcept {
        std::unique_lock<std::mutex> lock(mtx);
        destroyChildRec(reg, ent);
    }

    void loadScene(void(*newScene)(World&)) noexcept {
        needClean = true;
        newSceneFunc = newScene;
    }

private:
    const std::vector<std::string> getTypes(const Ent ent) noexcept {
        std::unique_lock<std::mutex> lock(mtx);
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
    void upgrade(World& world, Registry& reg) noexcept {
        for (const auto& pair: addComps) {
            for (const auto& pairType: pair.second) {
                reg.add(pair.first, pairType.second);
            }
        }

        for (const auto& pair: delComps) {
            for (const auto& type: pair.second) {
                reg.del(pair.first, type);
            }
        }

        for (const auto& pair: addEnts) {
            reg.newEnt(pair.first, pair.second);
        }

        for (const Ent ent: delEnts) {
            reg.destroy(ent);
        }

        delEnts.clear();
        addEnts.clear();
        delComps.clear();
        addComps.clear();

        if (needClean) {
            needClean = false;
            for (auto [dontDestroyEnt]: reg.view({typeid(DontDestroyOnLoad).hash_code()}, {})) {
                auto* arch = reg.entArch.at(dontDestroyEnt);
                std::unordered_map<Type, std::any> comps;
                for (auto& pair: arch->pools) {
                    comps.emplace(pair.first, pair.second->comps.at(dontDestroyEnt));
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
                    newChildrens.emplace(oldToNewEnts.at(oldChildEnt));
                }
                reg.appendChildren(newEntId, newChildrens);
            }
            dontDestroyesHierarchies.clear();
            newSceneFunc(world);
        }
    }

private:
    std::mutex mtx;
    std::unordered_map<Ent, std::unordered_map<Type, std::any>> addEnts;
    std::unordered_map<Ent, std::unordered_map<Type, std::any>> addComps;
    std::unordered_set<Ent> delEnts;
    std::unordered_map<Ent, std::unordered_set<Type>> delComps;
    std::unordered_map<Ent, std::unordered_map<Type, std::any>> dontDestroyes;
    std::unordered_map<Ent, std::unordered_set<Ent>> dontDestroyesHierarchies;
    bool needClean = false;
    void(*newSceneFunc)(World&);
};

///////////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////////

class World;

class ThreadPool final {
friend class Sys;
private:
    ThreadPool(World& newWorld, std::size_t newNbThreads) noexcept:
        world(newWorld),
        nbTasksDone(0),
        nbTasks(0),
        nbThreads(newNbThreads),
        isStop(false) {
        for (std::size_t i = 0; i < nbThreads; i++) {
            threads.emplace_back(std::bind(&ThreadPool::task, this));
        }
    }

private:
    ~ThreadPool() noexcept {
        stop();
        for (auto& thread: threads) {
            thread.join();
        }
    }

    void addTasks(const std::vector<void(*)(ThreadedSystem, World&)>& newTasks) noexcept {
        tasks.emplace_back(newTasks);
    }

    void addFixedTasks(const std::vector<void(*)(ThreadedFixedSystem, World&)>& newTasks) noexcept {
        fixedTasks.emplace_back(newTasks);
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
            if (!tasks.empty() && nbTasksDone != 0) {
               cvTask.notify_all(); 
            }
            return (fixedTasks.empty() && (nbTasks == 0));
        });
    }

private:
    void task() noexcept {
        srand(time(NULL));
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
            } else {
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
            }
        }
    }

    void stop() noexcept {
        std::unique_lock<std::mutex> lock(mtx);
        isStop = true;
        cvTask.notify_all();
    }

private:
    World& world;
    std::vector<std::vector<void(*)(ThreadedSystem, World&)>> tasks;
    std::vector<std::vector<void(*)(ThreadedFixedSystem, World&)>> fixedTasks;
    std::mutex mtx;
    std::size_t nbTasksDone;
    std::condition_variable cvTask;
    std::condition_variable cvFinished;
    std::vector<std::thread> threads;
    std::size_t nbTasks;
    std::size_t nbThreads;
    bool isStop;
};

///////////////////////////////////////////////////////////////////////////////////

class Sys final {
friend class World;
friend class ZerEngine;
private:
    Sys(World& world) noexcept:
        threadpool(world, std::thread::hardware_concurrency() - 1),
        isUseMultithreading(true)
    {
        srand(time(NULL));
    }

private:
    constexpr void useMultithreading(bool newVal) noexcept {
        isUseMultithreading = newVal;
    }

    constexpr void addStartSys(void(*const func)(StartSystem, World&)) noexcept {
        startSystems.emplace_back(func);
    }

    template <typename... Funcs>
    constexpr void addMainCondSys(bool(*const cond)(World&), Funcs&&... funcs) noexcept {
        mainSystems.emplace_back(cond, std::initializer_list<void(*)(MainSystem, World&)>{std::forward<Funcs>(funcs)...});
    }

    template <typename... Funcs>
    constexpr void addMainFixedCondSys(bool(*const cond)(World&), Funcs&&... funcs) noexcept {
        mainFixedSystems.emplace_back(cond, std::initializer_list<void(*)(MainFixedSystem, World&)>{std::forward<Funcs>(funcs)...});
    }

    template <typename... Funcs>
    constexpr void addThreadedCondSys(bool(*const cond)(World&), Funcs&&... funcs) noexcept {
        threadedSystems.emplace_back(cond, std::initializer_list<void(*)(ThreadedSystem, World&)>{std::forward<Funcs>(funcs)...});
    }

    template <typename... Funcs>
    constexpr void addThreadedFixedCondSys(bool(*const cond)(World&), Funcs&&... funcs) noexcept {
        threadedFixedSystems.emplace_back(cond, std::initializer_list<void(*)(ThreadedFixedSystem, World&)>{std::forward<Funcs>(funcs)...});
    }

    template <typename... Funcs>
    constexpr void addLateCondSys(bool(*const cond)(World&), Funcs&&... funcs) noexcept {
        lateSystems.emplace_back(cond, std::initializer_list<void(*)(LateSystem, World&)>{std::forward<Funcs>(funcs)...});
    }

    template <typename... Funcs>
    constexpr void addLateFixedCondSys(bool(*const cond)(World&), Funcs&&... funcs) noexcept {
        lateFixedSystems.emplace_back(cond, std::initializer_list<void(*)(LateFixedSystem, World&)>{std::forward<Funcs>(funcs)...});
    }

    void start(World& world) const noexcept {
        for (const auto& func: startSystems) {
            func({}, world);
        }
    }

    void run(World& world) noexcept {
        for (const auto& mainFunc: mainSystems) {
            if (mainFunc.first == nullptr || mainFunc.first(world)) {
                for (const auto& mainRow: mainFunc.second) {
                    mainRow({}, world);
                }
            }
        }

        for (const auto& funcs: threadedSystems) {
            if (funcs.first == nullptr || funcs.first(world)) {
                if (!isUseMultithreading) {
                    for (auto& func: funcs.second) {
                        func({}, world);
                    }
                } else {
                    threadpool.addTasks(funcs.second);
                }
            }
        }

        threadpool.run();

        threadpool.wait();
    }

    void runLate(World& world) noexcept {
        for (const auto& lateFunc: lateSystems) {
            if (lateFunc.first == nullptr || lateFunc.first(world)) {
                for (const auto& lateRow: lateFunc.second) {
                    lateRow({}, world);
                }
            }
        }
    }

    void runFixed(World& world) noexcept {
        for (const auto& mainFunc: mainFixedSystems) {
            if (mainFunc.first == nullptr || mainFunc.first(world)) {
                for (const auto& mainRow: mainFunc.second) {
                    mainRow({}, world);
                }
            }
        }

        for (const auto& funcs: threadedFixedSystems) {
            if (funcs.first == nullptr || funcs.first(world)) {
                if (!isUseMultithreading) {
                    for (auto& func: funcs.second) {
                        func({}, world);
                    }
                } else {
                    threadpool.addFixedTasks(funcs.second);
                }
            }
        }

        threadpool.fixedRun();

        threadpool.fixedWait();

        for (const auto& lateFunc: lateFixedSystems) {
            if (lateFunc.first == nullptr || lateFunc.first(world)) {
                for (const auto& lateRow: lateFunc.second) {
                    lateRow({}, world);
                }
            }
        }
    }

private:
    std::vector<void(*)(StartSystem, World&)> startSystems;
    std::vector<std::pair<bool(*)(World&), std::vector<void(*)(MainSystem, World&)>>> mainSystems;
    std::vector<std::pair<bool(*)(World&), std::vector<void(*)(MainFixedSystem, World&)>>> mainFixedSystems;
    std::vector<std::pair<bool(*)(World&), std::vector<void(*)(ThreadedSystem, World&)>>> threadedSystems;
    std::vector<std::pair<bool(*)(World&), std::vector<void(*)(ThreadedFixedSystem, World&)>>> threadedFixedSystems;
    std::vector<std::pair<bool(*)(World&), std::vector<void(*)(LateSystem, World&)>>> lateSystems;
    std::vector<std::pair<bool(*)(World&), std::vector<void(*)(LateFixedSystem, World&)>>> lateFixedSystems;
    ThreadPool threadpool;
    bool isUseMultithreading;
};

///////////////////////////////////////////////////////////////////////////////////

class Time final {
friend class ZerEngine;
public:
    Time(float newFixedTimeStep = 0.02f) noexcept:
        t2(std::chrono::high_resolution_clock::now()),
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
    [[nodiscard]] constexpr float delta() const noexcept {
        return dt * timeScale;
    }

    [[nodiscard]] constexpr float unscaledDelta() const noexcept {
        return dt;
    }

    [[nodiscard]] constexpr float unscaledFixedDelta() const noexcept {
        return fixedTimeStep;
    }

    [[nodiscard]] constexpr float fixedDelta() const noexcept {
        return fixedTimeStep * timeScale;
    }

    [[nodiscard]] constexpr bool isTimeStep() const noexcept {
        return isTimeStepFrame;
    }

    [[nodiscard]] constexpr unsigned int getNbFixedSteps() const noexcept {
        return nbFixedSteps;
    }

    constexpr float getTimeScale() const noexcept {
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
                printf("FPS: %zu\n", nbFrames);
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
    bool exist(const Ent ent) const noexcept {
        return reg.exist(ent);
    }

    template <typename T, typename... Ts>
    bool has(const Ent ent) const noexcept {
        if (!reg.exist(ent)) {
            return false;
        } else if (reg.has(ent, {typeid(T).hash_code()})) {
            if constexpr (sizeof...(Ts) > 0) {
                return has<Ts...>(ent);
            }
            return true;
        } else if (auto addEntsIt = lateUpgrade.addEnts.find(ent); addEntsIt != lateUpgrade.addEnts.end() && addEntsIt->second.contains(typeid(T).hash_code())) {
            if constexpr (sizeof...(Ts) > 0) {
                return has<Ts...>(ent);
            }
            return true;
        } else if (auto addCompsIt = lateUpgrade.addComps.find(ent); addCompsIt != lateUpgrade.addComps.end() && addCompsIt->second.contains(typeid(T).hash_code())) {
            if constexpr (sizeof...(Ts) > 0) {
                return has<Ts...>(ent);
            }
            return true;
        }
        return false;
    }

    const std::vector<std::string> getTypes(const Ent ent) noexcept {
        auto types = reg.getTypes(ent);
        types.append_range(lateUpgrade.getTypes(ent));
        return types;
    }

private:
    template <typename T>
    [[nodiscard]] std::optional<std::reference_wrapper<T>> internalGet(const Ent ent) noexcept {
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
    [[nodiscard]] std::optional<const std::reference_wrapper<T>> internalGet(const Ent ent) const noexcept {
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
    template <typename T, typename... Ts>
    [[nodiscard]] std::optional<std::tuple<T&, Ts&...>> get(const Ent ent) noexcept {
        if (auto opt = internalGet<T>(ent)) {
            if constexpr (sizeof...(Ts) > 0) {
                if (auto othOpt = get<Ts...>(ent)) {
                    return std::tuple_cat(std::forward_as_tuple(opt.value()), othOpt.value());
                } else {
                    return std::nullopt;
                }
            } else {
                return std::forward_as_tuple(opt.value());
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] bool hasParent(const Ent childEnt) const noexcept {
        return reg.hasParent(childEnt);
    }

    [[nodiscard]] std::optional<Ent> getParent(const Ent childEnt) const noexcept {
        return reg.getParent(childEnt);
    }

    [[nodiscard]] bool hasChildren(const Ent parentEnt) const noexcept {
        return reg.hasChildren(parentEnt);
    }

    [[nodiscard]] std::optional<std::reference_wrapper<const std::unordered_set<Ent>>> getChildren(const Ent parentEnt) const noexcept {
        return reg.getChildren(parentEnt);
    }

    void appendChildren(const Ent parentEnt, const std::vector<Ent>& childrenEnt) noexcept {
        if (has<DontDestroyOnLoad>(parentEnt)) {
            for (auto childEnt: childrenEnt) {
                addDontDestroyOnLoad(childEnt);
            }
        }
        if (has<IsInactive>(parentEnt)) {
            for (auto childEnt: childrenEnt) {
                setInactive(childEnt);
            }
        }
        reg.appendChildren(parentEnt, childrenEnt);
    }

    template <typename... Ts> requires (sizeof...(Ts) > 0)
    [[nodiscard]] decltype(auto) resource(this auto&& self) noexcept {
        return std::forward_as_tuple(std::any_cast<Ts&>(std::move(self).res.get(typeid(Ts).hash_code()))...);
    }

    [[nodiscard]] const std::unordered_set<Ent>& getDestroyedEnts() const noexcept {
        return lateUpgrade.delEnts;
    }

    [[nodiscard]] const std::unordered_map<Ent, std::unordered_map<Type, std::any>>& getAddedEnts() const noexcept {
        return lateUpgrade.addEnts;
    }

    [[nodiscard]] const std::unordered_map<Ent, std::unordered_map<Type, std::any>>& getAddedComps() const noexcept {
        return lateUpgrade.addComps;
    }

    [[nodiscard]] const std::unordered_map<Ent, std::unordered_set<Type>>& getDestroyedComps() const noexcept {
        return lateUpgrade.delComps;
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    [[nodiscard]] const View<Comps...> view(const With<Filters...>& filters = {}, const Without<Excludes...>& excludes = {}) noexcept {
        return reg.view<Comps...>(
            {typeid(Comps).hash_code()..., typeid(Filters).hash_code()...},
            {typeid(IsInactive).hash_code(), typeid(Excludes).hash_code()...}
        );
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    [[nodiscard]] const View<Comps...> view(const Without<Excludes...>& excludes, const With<Filters...>& filters = {}) noexcept {
        return reg.view<Comps...>(
            {typeid(Comps).hash_code()..., typeid(Filters).hash_code()...},
            {typeid(IsInactive).hash_code(), typeid(Excludes).hash_code()...}
        );
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    [[nodiscard]] const View<Comps...> view(const With<Filters...>& filters, const Without<Excludes...>& excludes, const WithInactive&) noexcept {
        return reg.view<Comps...>(
            {typeid(Comps).hash_code()..., typeid(Filters).hash_code()...},
            {typeid(Excludes).hash_code()...}
        );
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    [[nodiscard]] const View<Comps...> view(const Without<Excludes...>& excludes, const With<Filters...>& filters, const WithInactive&) noexcept {
        return reg.view<Comps...>(
            {typeid(Comps).hash_code()..., typeid(Filters).hash_code()...},
            {typeid(Excludes).hash_code()...}
        );
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    [[nodiscard]] const View<Comps...> view(const With<Filters...>& filters, const WithInactive&, const Without<Excludes...>& excludes = {}) noexcept {
        return reg.view<Comps...>(
            {typeid(Comps).hash_code()..., typeid(Filters).hash_code()...},
            {typeid(Excludes).hash_code()...}
        );
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    [[nodiscard]] const View<Comps...> view(const Without<Excludes...>& excludes, const WithInactive&, const With<Filters...>& filters = {}) noexcept {
        return reg.view<Comps...>(
            {typeid(Comps).hash_code()..., typeid(Filters).hash_code()...},
            {typeid(Excludes).hash_code()...}
        );
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    [[nodiscard]] const View<Comps...> view(const WithInactive&, const With<Filters...>& filters = {}, const Without<Excludes...>& excludes = {}) noexcept {
        return reg.view<Comps...>(
            {typeid(Comps).hash_code()..., typeid(Filters).hash_code()...},
            {typeid(Excludes).hash_code()...}
        );
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    [[nodiscard]] const View<Comps...> view(const WithInactive&, const Without<Excludes...>& excludes, const With<Filters...>& filters = {}) noexcept {
        return reg.view<Comps...>(
            {typeid(Comps).hash_code()..., typeid(Filters).hash_code()...},
            {typeid(Excludes).hash_code()...}
        );
    }

    template <typename... Comps> requires (std::copy_constructible<Comps> && ...)
    Ent newEnt(const Comps&... comps) noexcept {
        return lateUpgrade.newEnt(
            reg.getEntToken(),
            {{typeid(Comps).hash_code(), std::make_any<Comps>(comps)}...}
        );
    }

    template <typename Comp, typename... Comps> requires (std::copy_constructible<Comp>)
    std::optional<std::tuple<Comp&, Comps&...>> add(const Ent ent, const Comp& comp, const Comps&... comps) noexcept {
        if (reg.exist(ent)) {
            lateUpgrade.add(
                ent,
                std::initializer_list<std::pair<const Type, std::any>>{
                    {typeid(Comp).hash_code(), std::make_any<Comp>(comp)},
                    {typeid(Comps).hash_code(), std::make_any<Comps>(comps)}...
                }
            );
        } else {
            printf("World::add(): Impossible d'ajouter sur une entitée qui n'existe pas [type: %s]\n", typeid(Comp).name());
            (printf("World::add(): Impossible d'ajouter sur une entitée qui n'existe pas [type: %s]\n", typeid(Comps).name()), ...);
            return std::nullopt;
        }

        return std::forward_as_tuple(internalGet<Comp>(ent).value(), internalGet<Comps>(ent).value()...);
    }

    template <typename T, typename... Ts>
    void del(const Ent ent) noexcept {
        if (reg.has(ent, {typeid(T).hash_code()})) {
            lateUpgrade.del(ent, typeid(T).hash_code());
        } else {
            printf("World::del(): Impossible de supprimer un composant qui n'existe pas - %s\n", typeid(T).name());
        }

        if constexpr (sizeof...(Ts) > 0) {
            del<Ts...>(ent);
        }
    }

    void destroy(const Ent ent) noexcept {
        if (reg.exist(ent)) {
            lateUpgrade.destroy(reg, ent);
        } else {
            printf("World::destroy(): Impossible de supprimer une entitée qui n'existe pas\n");
        }
    }

    void setActive(const Ent ent) noexcept {
        if (has<IsInactive>(ent)) {
            del<IsInactive>(ent);
            if (auto childrenOpt = getChildren(ent)) {
                for (auto childEnt: childrenOpt.value().get()) {
                    setActive(childEnt);
                }
            }
        }
    }

    void setInactive(const Ent ent) noexcept {
        if (!has<IsInactive>(ent)) {
            add(ent, IsInactive());
            if (auto childrenOpt = getChildren(ent)) {
                for (auto childEnt: childrenOpt.value().get()) {
                    setInactive(childEnt);
                }
            }
        }
    }

    [[nodiscard]] constexpr std::size_t getTotalEntities() const noexcept {
        return reg.entArch.size();
    }

    void addDontDestroyOnLoad(const Ent ent) noexcept {
        if (!has<DontDestroyOnLoad>(ent)) {
            add(ent, DontDestroyOnLoad());
            if (auto childrenOpt = getChildren(ent)) {
                for (auto childEnt: childrenOpt.value().get()) {
                    addDontDestroyOnLoad(childEnt);
                }
            }
        }
    }

    void loadScene(void(*newScene)(World&)) noexcept {
        lateUpgrade.loadScene(newScene);
    }

    void stopRun(bool val = true) noexcept {
        isRunning = !val;
    }

    void upgrade() noexcept {
#ifdef ZER_DEBUG_INTEGRITY
    try {
#endif
        lateUpgrade.upgrade(*this, reg);
#ifdef ZER_DEBUG_INTEGRITY
    } catch(const std::exception& except) {
        printf("World::%s: %s\n", __func__, except.what());
    }
#endif
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
    ZerEngine() {
        world.res.emplace(typeid(Time).hash_code(), std::make_any<Time>(0.02f));
    }

    [[nodiscard]] constexpr ZerEngine& useMultithreading(bool newVal) noexcept {
        world.sys.useMultithreading(newVal);
        return *this;
    }

    [[nodiscard]] constexpr ZerEngine& setFixedTimeStep(float newFixedTimeStep) noexcept {
        auto [time] = world.resource<Time>();
        time.setFixedTimeStep(newFixedTimeStep);
        return *this;
    }

    template <typename T, typename... Args> requires (std::copy_constructible<T>)
    [[nodiscard]] ZerEngine& addRes(Args&&... args) noexcept {
        world.res.emplace(typeid(T).hash_code(), std::make_any<T>(std::forward<Args>(args)...));
        return *this;
    }

    template <typename T, typename... Args>
    [[nodiscard]] ZerEngine& addPlugin(void(*pluginFunc)(ZerEngine&)) noexcept {
        pluginFunc(*this);
        return *this;
    }

    [[nodiscard]] ZerEngine& addStartSys(void(*const func)(StartSystem, World&)) noexcept {
        world.sys.addStartSys(func);
        return *this;
    }

    template <typename... Funcs> requires ((std::same_as<Funcs, void(&)(MainSystem, World&)> || std::same_as<Funcs, void(&)(MainSystem, World&) noexcept>) && ...)
    [[nodiscard]] ZerEngine& addMainSys(Funcs&&... funcs) noexcept {
        world.sys.addMainCondSys(nullptr, std::forward<Funcs>(funcs)...);
        return *this;
    }

    template <typename... Funcs> requires ((std::same_as<Funcs, void(&)(MainSystem, World&)> || std::same_as<Funcs, void(&)(MainSystem, World&) noexcept>) && ...)
    [[nodiscard]] ZerEngine& addMainCondSys(bool(*const cond)(World&), Funcs&&... funcs) noexcept {
        world.sys.addMainCondSys(cond, std::forward<Funcs>(funcs)...);
        return *this;
    }

    template <typename... Funcs> requires ((std::same_as<Funcs, void(&)(MainFixedSystem, World&)> || std::same_as<Funcs, void(&)(MainFixedSystem, World&) noexcept>) && ...)
    [[nodiscard]] ZerEngine& addMainFixedSys(Funcs&&... funcs) noexcept {
        world.sys.addMainFixedCondSys(nullptr, std::forward<Funcs>(funcs)...);
        return *this;
    }

    template <typename... Funcs> requires ((std::same_as<Funcs, void(&)(MainFixedSystem, World&)> || std::same_as<Funcs, void(&)(MainFixedSystem, World&) noexcept>) && ...)
    [[nodiscard]] ZerEngine& addMainFixedCondSys(bool(*const cond)(World&), Funcs&&... funcs) noexcept {
        world.sys.addMainFixedCondSys(cond, std::forward<Funcs>(funcs)...);
        return *this;
    }

    template <typename... Funcs> requires ((std::same_as<Funcs, void(&)(ThreadedSystem, World&)> || std::same_as<Funcs, void(&)(ThreadedSystem, World&) noexcept>) && ...)
    [[nodiscard]] ZerEngine& addThreadedSys(Funcs&&... funcs) noexcept {
        world.sys.addThreadedCondSys(nullptr, std::forward<Funcs>(funcs)...);
        return *this;
    }

    template <typename... Funcs> requires ((std::same_as<Funcs, void(&)(ThreadedSystem, World&)> || std::same_as<Funcs, void(&)(ThreadedSystem, World&) noexcept>) && ...)
    [[nodiscard]] ZerEngine& addThreadedCondSys(bool(*const cond)(World&), Funcs&&... funcs) noexcept {
        world.sys.addThreadedCondSys(cond, std::forward<Funcs>(funcs)...);
        return *this;
    }

    template <typename... Funcs> requires ((std::same_as<Funcs, void(&)(ThreadedFixedSystem, World&)> || std::same_as<Funcs, void(&)(ThreadedFixedSystem, World&) noexcept>) && ...)
    [[nodiscard]] ZerEngine& addThreadedFixedSys(Funcs&&... funcs) noexcept {
        world.sys.addThreadedFixedCondSys(nullptr, std::forward<Funcs>(funcs)...);
        return *this;
    }

    template <typename... Funcs> requires ((std::same_as<Funcs, void(&)(ThreadedFixedSystem, World&)> || std::same_as<Funcs, void(&)(ThreadedFixedSystem, World&) noexcept>) && ...)
    [[nodiscard]] ZerEngine& addThreadedFixedCondSys(bool(*const cond)(World&), Funcs&&... funcs) noexcept {
        world.sys.addThreadedFixedCondSys(cond, std::forward<Funcs>(funcs)...);
        return *this;
    }

    template <typename... Funcs> requires ((std::same_as<Funcs, void(&)(LateSystem, World&)> || std::same_as<Funcs, void(&)(LateSystem, World&) noexcept>) && ...)
    [[nodiscard]] ZerEngine& addLateSys(Funcs&&... funcs) noexcept {
        world.sys.addLateCondSys(nullptr, std::forward<Funcs>(funcs)...);
        return *this;
    }

    template <typename... Funcs> requires ((std::same_as<Funcs, void(&)(LateSystem, World&)> || std::same_as<Funcs, void(&)(LateSystem, World&) noexcept>) && ...)
    [[nodiscard]] ZerEngine& addLateCondSys(bool(*const cond)(World&), Funcs&&... funcs) noexcept {
        world.sys.addLateCondSys(cond, std::forward<Funcs>(funcs)...);
        return *this;
    }

    template <typename... Funcs> requires ((std::same_as<Funcs, void(&)(LateFixedSystem, World&)> || std::same_as<Funcs, void(&)(LateFixedSystem, World&) noexcept>) && ...)
    [[nodiscard]] ZerEngine& addLateFixedSys(Funcs&&... funcs) noexcept {
        world.sys.addLateFixedCondSys(nullptr, std::forward<Funcs>(funcs)...);
        return *this;
    }

    template <typename... Funcs> requires ((std::same_as<Funcs, void(&)(LateFixedSystem, World&)> || std::same_as<Funcs, void(&)(LateFixedSystem, World&) noexcept>) && ...)
    [[nodiscard]] ZerEngine& addLateFixedCondSys(bool(*const cond)(World&), Funcs&&... funcs) noexcept {
        world.sys.addLateFixedCondSys(cond, std::forward<Funcs>(funcs)...);
        return *this;
    }

    void run() noexcept {
        world.isRunning = true;
        world.sys.start(world);
        while (world.isRunning) {
            world.upgrade();

            auto [time] = world.resource<Time>();
            time.update();

            world.sys.run(world);

            if (time.isTimeStep()) {
                for (unsigned int i = 0; i < time.getNbFixedSteps(); i++) {
                    world.upgrade();
                    world.sys.runFixed(world);
                }
            }

            world.sys.runLate(world);
        }
        world.upgrade();
    }

private:
    World world;
};

///////////////////////////////////////////////////////////////////////////////////
