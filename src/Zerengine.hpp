#pragma once

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
#include <memory>
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

static constexpr inline std::size_t ZERENGINE_VERSION_MAJOR = 24;
static constexpr inline std::size_t ZERENGINE_VERSION_MINOR = 10;
static constexpr inline std::size_t ZERENGINE_VERSION_PATCH = 4;

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

class IComponent {
protected:
    constexpr IComponent() noexcept = default;

public:
    constexpr virtual ~IComponent() noexcept = default;
};

class IsInactive final: public IComponent {};
class DontDestroyOnLoad final: public IComponent {};

class IResource {
protected:
    constexpr IResource() noexcept = default;

public:
    constexpr virtual ~IResource() noexcept = default;
};

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

template <typename T>
concept IsComponentConcept = [] -> bool {
    static_assert(std::is_class_v<T>, "Impossible d'ajouter un Composant qui ne soit pas une Classe");
    static_assert(std::derived_from<T, IComponent>, "Impossible d'ajouter un Composant qui n'implemente pas IComponent");
    static_assert(std::is_final_v<T>, "Impossible d'ajouter un Composant qui ne soit pas Final");
    static_assert(std::copy_constructible<T>, "Impossible d'ajouter un Composant qui ne soit pas Copiable");
    return true;
}();

template <typename T>
concept IsResourceConcept = [] -> bool {
    static_assert(std::is_class_v<T>, "Impossible d'ajouter une Ressource qui ne soit pas une Classe");
    static_assert(std::derived_from<T, IResource>, "Impossible d'ajouter une Ressource qui n'implemente pas IComponent");
    static_assert(std::is_final_v<T>, "Impossible d'ajouter une Ressource qui ne soit pas Final");
    static_assert(std::copy_constructible<T>, "Impossible d'ajouter une Ressource qui ne soit pas Copiable");
    return true;
}();

///////////////////////////////////////////////////////////////////////////////////

class CompPool final {
// friend class Archetype;
friend class LateUpgrade;
public:
    [[nodiscard]] CompPool() noexcept = default;
    [[nodiscard]] CompPool(const Ent& entity, std::shared_ptr<IComponent>&& component) noexcept:
        components({{entity, std::move(component)}}) {
    }

public:
    constexpr auto insert_entity(this auto& self, const Ent& entity, std::shared_ptr<IComponent>&& component) noexcept -> void {
        self.components.emplace(entity, std::move(component));
    }

    [[nodiscard]] constexpr auto get_entity(this auto& self, const Ent& entity) noexcept -> auto& {
        return self.components.at(entity);
    }

    [[nodiscard]] constexpr auto contains_entity(this const auto& self, const Ent& entity) noexcept -> bool {
        return self.components.contains(entity);
    }

    [[nodiscard]] auto remove_entity(this auto& self, const Ent& entity) noexcept -> std::shared_ptr<IComponent> {
        auto component = self.components.at(entity);
        self.components.erase(entity);
        return component;
    }

private:
    std::unordered_map<Ent, std::shared_ptr<IComponent>> components;
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
template <typename... Ts>
friend class View;
public:
    Archetype() noexcept = default;

    Archetype(const Ent& ent, std::unordered_map<Type, std::shared_ptr<IComponent>>&& components) noexcept:
        ents({ent}),
        pools(components.size()) {
        for (auto&& [type, component]: components) {
            if (component) {
                pools.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(type),
                    std::forward_as_tuple(std::make_unique<CompPool>(ent, std::move(component)))
                );
            } else {
                pools.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(type),
                    std::forward_as_tuple(nullptr)
                );
            }
        }
    }

    Archetype(ArchetypeCreateWith, std::shared_ptr<Archetype>& oldArch, const Ent& ent, std::pair<const Type, std::shared_ptr<IComponent>>&& component) noexcept:
        ents({ent}),
        pools(oldArch->pools.size() + 1) {
        for (auto&& [old_type, old_component]: oldArch->pools) {
            if (old_component) {
                pools.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(old_type),
                    std::forward_as_tuple(std::make_unique<CompPool>(ent, std::move(old_component->remove_entity(ent))))
                );
            } else {
                pools.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(old_type),
                    std::forward_as_tuple(nullptr)
                );
            }
        }
        if (component.second) {
            pools.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(component.first),
                std::forward_as_tuple(std::make_unique<CompPool>(ent, std::move(component.second)))
            );
        } else {
            pools.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(component.first),
                std::forward_as_tuple(nullptr)
            );
        }
        oldArch->destroy(ent);
    }

    Archetype(ArchetypeCreateWithout, std::shared_ptr<Archetype>& oldArch, const Ent& ent, const Type& component_type) noexcept:
        ents({ent}),
        pools(oldArch->pools.size() - 1) {
        for (auto&& [old_type, old_component]: oldArch->pools) {
            if (old_type != component_type) {
                if (old_component) {
                    pools.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(old_type),
                        std::forward_as_tuple(std::make_unique<CompPool>(ent, std::move(old_component->remove_entity(ent))))
                    );
                } else {
                    pools.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(old_type),
                        std::forward_as_tuple(nullptr)
                    );
                }
            }
        }
        oldArch->destroy(ent);
    }

private:
    constexpr auto new_entity(this auto& self, const Ent& entity, std::unordered_map<Type, std::shared_ptr<IComponent>>&& components) noexcept -> void {
        self.ents.emplace(entity);
        for (auto&& [type, component]: components) {
            if (component) {
                self.pools.at(type)->insert_entity(entity, std::move(component));
            }
        }
    }

    auto add_component(this auto& self, const Ent& entity, std::shared_ptr<Archetype>& oldArch, std::pair<const Type, std::shared_ptr<IComponent>>&& component) noexcept -> void {
        self.ents.emplace(entity);
        for (auto& [old_type, old_component_pool]: oldArch->pools) {
            if (old_component_pool) {
                self.pools.at(old_type)->insert_entity(entity, std::move(old_component_pool->remove_entity(entity)));
            }
        }
        if (component.second) {
            self.pools.at(component.first)->insert_entity(entity, std::move(component.second));
        }
        oldArch->destroy(entity);
    }

    auto remove_component(this auto& self, const Ent& entity, std::shared_ptr<Archetype>& oldArch, const Type& component_type) noexcept -> void {
        self.ents.emplace(entity);
        for (auto& [old_type, old_component_pool]: oldArch->pools) {
            if (old_type != component_type && old_component_pool) {
                self.pools.at(old_type)->insert_entity(entity, std::move(old_component_pool->remove_entity(entity)));
            }
        }
        oldArch->destroy(entity);
    }

    [[nodiscard]] constexpr auto get(this auto& self, const Ent& entity, const Type& component_type) noexcept -> auto& {
        return self.pools.at(component_type)->get_entity(entity);
    }

    auto destroy(this auto& self, const Ent& entity) noexcept -> void {
        self.ents.erase(entity);
        for (auto& [_, comp_pool]: self.pools) {
            if (comp_pool && comp_pool->contains_entity(entity)) {
                static_cast<void>(comp_pool->remove_entity(entity));
            }
        }
    }

    [[nodiscard]] constexpr auto empty(this const auto& self) noexcept -> bool {
        return self.ents.empty();
    }

    [[nodiscard]] constexpr auto size(this const auto& self) noexcept -> std::size_t {
        return self.ents.size();
    }

    [[nodiscard]] constexpr auto containsType(this const auto& self, const Type& type) noexcept -> bool {
        return self.pools.contains(type);
    }

private:
    template <typename... Ts>
    [[nodiscard]] constexpr auto getTupleWithEnt(this auto& self, const Ent& entity) noexcept -> std::tuple<const Ent&, Ts&...> {
        return std::forward_as_tuple(entity, (*static_cast<Ts*>(self.get(entity, typeid(Ts).hash_code()).get()))...);
    }

private:
    [[nodiscard]] constexpr auto isTotalyCompatibleLate(this const auto& self, const std::unordered_map<Type, std::shared_ptr<IComponent>>& components) noexcept -> bool {
        if (components.size() != self.pools.size()) {
            return false;
        }
        for (const auto& [type, _]: components) {
            if (!self.pools.contains(type)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] auto isTotalyCompatibleLate(this const auto& self, const std::shared_ptr<Archetype>& oldArch, const Type& component_type) noexcept -> bool {
        if (oldArch->pools.size() + 1 != self.pools.size() || !self.pools.contains(component_type)) {
            return false;
        }
        for (const auto& [old_type, _]: oldArch->pools) {
            if (!self.pools.contains(old_type)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] auto isTotalyCompatibleWithoutLate(this const auto& self, const std::shared_ptr<Archetype>& oldArch, const Type& component_type) noexcept -> bool {
        if (oldArch->pools.size() - 1 != self.pools.size() || self.pools.contains(component_type)) {
            return false;
        }
        for (const auto& [old_type, _]: oldArch->pools) {
            if (!self.pools.contains(old_type) && old_type != component_type) {
                return false;
            }
        }
        return true;
    }

private:
    std::unordered_set<Ent> ents;
    std::unordered_map<Type, std::unique_ptr<CompPool>> pools;
};

///////////////////////////////////////////////////////////////////////////////////

template <typename... Ts>
class View final {
friend class Registry;
friend class LiteRegistry;
private:
    constexpr View(std::unordered_set<std::shared_ptr<Archetype>>&& newArchs) noexcept:
        archs(std::move(newArchs)) {
    }

public:
    [[nodiscard]] constexpr auto empty() const noexcept -> bool {
        if (archs.empty()) {
            return true;
        }
        for (const auto& arch: archs) {
            if (arch->size() > 0) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr auto size() const noexcept -> std::size_t {
        std::size_t newSize = 0;
        for (const auto& arch: archs) {
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
        ViewIterator(const std::unordered_set<std::shared_ptr<Archetype>>& newArchs, std::unordered_set<std::shared_ptr<Archetype>>::const_iterator newArchsIt) noexcept:
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
        std::unordered_set<std::shared_ptr<Archetype>>::const_iterator archsIt;
        std::unordered_set<Ent>::iterator entsIt;
        const std::unordered_set<std::shared_ptr<Archetype>>& archs;
    };

public:
    [[nodiscard]] constexpr auto begin() const noexcept -> ViewIterator{
        return {archs, archs.begin()};
    }

    [[nodiscard]] constexpr auto end() const noexcept -> ViewIterator {
        return {archs, archs.end()};
    }

private:
    const std::unordered_set<std::shared_ptr<Archetype>> archs;
};

///////////////////////////////////////////////////////////////////////////////////

class Registry final {
friend class World;
friend class LateUpgrade;
private:
    Registry() noexcept:
        emptyArch(std::make_shared<Archetype>()) {
    }

private:
    [[nodiscard]] constexpr auto getEntToken() noexcept -> Ent {
        Ent ent = lastEnt++;

        if (!entTokens.empty()) {
            lastEnt--;
            ent = entTokens.back();
            entTokens.pop_back();
        }

        entArch.emplace(ent, emptyArch);

        return ent;
    }

    auto newEnt(const Ent& entity, std::unordered_map<Type, std::shared_ptr<IComponent>>&& components) noexcept -> void {
        auto entArchIt = entArch.find(entity);
        if (entArchIt->second->size() > 0) {
            std::println("ZerEngine: Impossible d'inserer une entité deja existante - [{}]", entity);
            return;
        }

        std::unordered_set<std::shared_ptr<Archetype>> compatiblesArchs(archs);
        for (const auto& [type, _]: components) {
            filterArchsByType(type, compatiblesArchs);
        }
        for (auto& arch: compatiblesArchs) {
            if (arch->isTotalyCompatibleLate(components)) {
                arch->new_entity(entity, std::move(components));
                entArchIt->second = arch;
                return;
            }
        }

        std::shared_ptr<Archetype> arch = std::make_shared<Archetype>(entity, std::move(components));
        archs.emplace(arch);
        entArchIt->second = arch;
        for (const auto& [type, _]: arch->pools) {
            emplaceArchByType(type, arch);
        }
    }

    auto add(const Ent& entity, std::pair<const Type, std::shared_ptr<IComponent>>&& component) noexcept -> void {
        auto entArchIt = entArch.find(entity);
        if (entArchIt == entArch.end()) {
            std::println("ZerEngine - Registry: Impossible d'ajouter un composant sur une entité inexistante - [{}]", entity);
            return;
        }

        if (entArchIt->second->containsType(component.first)) {
            std::println("ZerEngine - Registry: Impossible d'ajouter 2 composants identiques  - [{}]", entity);
            return;
        }

        std::shared_ptr<Archetype> oldArch = entArchIt->second;

        std::unordered_set<std::shared_ptr<Archetype>> compatiblesArchs(archs);
        for (const auto& pairPools: oldArch->pools) {
            filterArchsByType(pairPools.first, compatiblesArchs);
        }
        filterArchsByType(component.first, compatiblesArchs);
        for (auto& arch: compatiblesArchs) {
            if (arch->isTotalyCompatibleLate(oldArch, component.first)) {
                arch->add_component(entity, oldArch, std::move(component));
                entArchIt->second = arch;
                emplaceArchByType(component.first, arch);
                removeOldArchIfEmpty(oldArch);
                return;
            }
        }

        std::shared_ptr<Archetype> arch = std::make_shared<Archetype>(archetypeCreateWith, oldArch, entity, std::move(component));
        archs.emplace(arch);
        entArchIt->second = arch;
        for (const auto& [type, _]: arch->pools) {
            emplaceArchByType(type, arch);
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

        std::shared_ptr<Archetype> oldArch = entArchIt->second;

        std::unordered_set<std::shared_ptr<Archetype>> compatiblesArchs(archs);
        for (const auto& pairPools: oldArch->pools) {
            if (pairPools.first != component_type) {
                filterArchsByType(pairPools.first, compatiblesArchs);
            }
        }
        for (auto& arch: compatiblesArchs) {
            if (arch->isTotalyCompatibleWithoutLate(oldArch, component_type)) {
                arch->remove_component(entity, oldArch, component_type);
                entArchIt->second = arch;
                removeOldArchIfEmpty(oldArch);
                return;
            }
        }

        std::shared_ptr<Archetype> arch = std::make_shared<Archetype>(archetypeCreateWithout, oldArch, entity, component_type);
        archs.emplace(arch);
        entArchIt->second = arch;
        for (const auto& [type, _]: arch->pools) {
            emplaceArchByType(type, arch);
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

    [[nodiscard]] constexpr auto get(this auto& self, const Ent& entity, const Type& component_type) noexcept -> auto& {
        return self.entArch.at(entity)->get(entity, component_type);
    }

    auto destroy(const Ent& entity) noexcept -> void {
        auto entArchIt = entArch.find(entity);
        if (entArchIt == entArch.end()) {
            std::println("ZerEngine: Impossible de detruire une entitée qui n'existe pas");
            return;
        }

        std::shared_ptr<Archetype> arch = entArchIt->second;
        arch->destroy(entity);
        entArch.erase(entArchIt);
        removeOldArchIfEmpty(arch);
        entTokens.push_back(entity);
        detachChildren(entity);
        removeParent(entity);
    }

    auto clean() noexcept -> void {
        emptyArch.reset();
        emptyArch = std::make_shared<Archetype>();
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
                    add(childEnt, std::pair(typeid(IsInactive).hash_code(), std::make_shared<IsInactive>()));
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
                        add(childEnt, std::pair(typeid(IsInactive).hash_code(), std::make_shared<IsInactive>()));
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
                    add(childEnt, std::pair(typeid(DontDestroyOnLoad).hash_code(), std::make_shared<DontDestroyOnLoad>()));
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
                        add(childEnt, std::pair(typeid(DontDestroyOnLoad).hash_code(), std::make_shared<DontDestroyOnLoad>()));
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
        std::unordered_set<std::shared_ptr<Archetype>> internalArchs;
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
    constexpr auto viewAddComp(std::unordered_set<std::shared_ptr<Archetype>>& internalArchs, const std::initializer_list<Type>& compTypes) const noexcept -> void {
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
                    std::vector<std::shared_ptr<Archetype>> toErase;
                    for (auto& arch: internalArchs) {
                        if (!othArchs.contains(arch)) {
                            toErase.emplace_back(arch);
                        }
                    }
                    for (auto& arch: toErase) {
                        internalArchs.erase(arch);
                    }
                } else {
                    internalArchs.clear();
                    return;
                }
            }
        }
    }

    constexpr auto viewWithoutComp(std::unordered_set<std::shared_ptr<Archetype>>& internalArchs, const std::initializer_list<Type>& compTypes) const noexcept -> void {
        for (const auto compType: compTypes) {
            if (auto archsByTypeIt = archsByType.find(compType); archsByTypeIt != archsByType.end()) {
                for (auto& arch: archsByTypeIt->second) {
                    if (internalArchs.contains(arch)) {
                        internalArchs.erase(arch);
                    }
                }
            }
        }
    }

    auto filterArchsByType(const Type& component_type, std::unordered_set<std::shared_ptr<Archetype>>& compatibleArchs) noexcept -> void {
        std::unordered_set<std::shared_ptr<Archetype>> newArchs;
        if (auto archsByTypeIt = archsByType.find(component_type); archsByTypeIt != archsByType.end()) {
            for (auto& arch: archsByTypeIt->second) {
                if (compatibleArchs.contains(arch)) {
                    newArchs.emplace(arch);
                }
            }
        }
        compatibleArchs = newArchs;
    }

private:
    auto emplaceArchByType(const Type& component_type, const std::shared_ptr<Archetype>& arch) noexcept -> void {
        if (auto archsByTypeIt = archsByType.find(component_type); archsByTypeIt != archsByType.end()) {
            archsByTypeIt->second.emplace(arch);
        } else {
            archsByType.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(component_type),
                std::forward_as_tuple(std::initializer_list<std::shared_ptr<Archetype>>{arch})
            );
        }
    }

    auto removeOldArchIfEmpty(const std::shared_ptr<Archetype>& oldArch) noexcept -> void {
        if (oldArch->size() <= 0 && oldArch != emptyArch) {
            for (const auto& [old_type, _]: oldArch->pools) {
                archsByType.at(old_type).erase(oldArch);
            }
            archs.erase(oldArch);
        }
    }

private:
    Ent lastEnt = 1;
    std::vector<Ent> entTokens;
    std::shared_ptr<Archetype> emptyArch;
    std::unordered_set<std::shared_ptr<Archetype>> archs;
    std::unordered_map<Ent, std::shared_ptr<Archetype>> entArch;
    std::unordered_map<Type, std::unordered_set<std::shared_ptr<Archetype>>> archsByType;
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
    auto newEnt(const Ent& ent, std::initializer_list<std::pair<const Type, std::shared_ptr<IComponent>>>&& newList) noexcept -> Ent {
        const std::unique_lock<std::mutex> lock(mtx);
        addEnts.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(ent),
            std::forward_as_tuple(std::move(newList))
        );
        return ent;
    }

    void add(const Registry& reg, const Ent& ent, std::initializer_list<std::tuple<const Type, const char*, std::shared_ptr<IComponent>>>&& newList) noexcept {
        const std::unique_lock<std::mutex> lock(mtx);
        auto addCompsIt = addComps.find(ent);
        if (addCompsIt != addComps.end()) {
            for (auto&& [type, name, component]: newList) {
                if (!addCompsIt->second.contains(type)) {
                    addCompsIt->second.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(std::move(type)),
                        std::forward_as_tuple(std::move(component))
                    );
                } else {
                    std::println("LateUpgrade: No Add Sur Comp: Le Composant {} existe deja (dans addComp)", name);
                }
            }
        } else {
            addCompsIt = addComps.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(ent),
                std::forward_as_tuple()
            ).first;
            for (auto&& [type, name, component]: newList) {
                if (!reg.has(ent, {type})) {
                    if (!addCompsIt->second.contains(type)) {
                        addCompsIt->second.emplace(
                            std::piecewise_construct,
                            std::forward_as_tuple(std::move(type)),
                            std::forward_as_tuple(std::move(component))
                        );
                    } else {
                        std::println("LateUpgrade: No Add Sur Comp: Le Composant {} existe deja (en double sur l'ajout dans addComp)", name);
                    }
                } else {
                    std::println("LateUpgrade: No Add Sur Comp: Le Composant {} existe deja (sur l'entite)", name);
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
            reg.add(ent, std::pair(typeid(IsInactive).hash_code(), std::make_shared<IsInactive>()));
            if (auto childrenOpt = reg.getChildren(ent)) {
                for (auto childEnt: childrenOpt.value().get()) {
                    setInactiveRec(reg, childEnt);
                }
            }
        }
    }

    void addDontDetroyOnLoadRec(Registry& reg, const Ent& ent) {
        if (!reg.has(ent, {typeid(DontDestroyOnLoad).hash_code()})) {
            reg.add(ent, std::pair(typeid(DontDestroyOnLoad).hash_code(), std::make_shared<DontDestroyOnLoad>()));
            if (auto childrenOpt = reg.getChildren(ent)) {
                for (auto childEnt: childrenOpt.value().get()) {
                    addDontDetroyOnLoadRec(reg, childEnt);
                }
            }
        }
    }

private:
    void upgrade(World& world, Registry& reg) noexcept {
        for (auto&& [entity, components]: addEnts) {
            reg.newEnt(entity, std::move(components));
        }
        addEnts.clear();

        for (const auto& [ent, types]: delComps) {
            for (const auto& type: types) {
                reg.remove(ent, type);
            }
        }

        for (auto&& [entity, components]: addComps) {
            for (auto&& component: components) {
                reg.add(entity, std::move(component));
            }
        }
        addComps.clear();

        for (const auto& [parent_ent, children_ents]: addParentChildren) {
            reg.appendChildren(parent_ent, children_ents);
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
        delComps.clear();
        setActiveEnts.clear();
        setInactiveEnts.clear();
        addDontDestroyOnLoadEnts.clear();
        addParentChildren.clear();

        if (needClean) {
            needClean = false;
            std::unordered_map<Ent, std::unordered_set<Ent>> dontDestroyesHierarchies;
            for (auto [dontDestroyEnt]: reg.view({typeid(DontDestroyOnLoad).hash_code()}, {})) {
                std::shared_ptr<Archetype> arch = reg.entArch.at(dontDestroyEnt);
                std::unordered_map<Type, std::shared_ptr<IComponent>> comps;
                for (auto& [type, component_pool]: arch->pools) {
                    if (component_pool) {
                        comps.emplace(type, component_pool->remove_entity(dontDestroyEnt));
                    } else {
                        comps.emplace(type, nullptr);
                    }
                }
                dontDestroyes.emplace(dontDestroyEnt, comps);
                if (auto childrenIt = reg.parentChildrens.find(dontDestroyEnt); childrenIt != reg.parentChildrens.end()) {
                    dontDestroyesHierarchies.emplace(dontDestroyEnt, childrenIt->second);
                }
            }
            reg.clean();

            std::unordered_map<Ent, Ent> oldToNewEnts;
            for (auto&& [entity, components]: dontDestroyes) {
                auto newEntId = reg.getEntToken();
                reg.newEnt(newEntId, std::move(components));
                oldToNewEnts.emplace(entity, newEntId);
            }
            dontDestroyes.clear();

            for (const auto& [parent_ent, children_ents]: dontDestroyesHierarchies) {
                auto newEntId = oldToNewEnts.at(parent_ent);
                std::unordered_set<Ent> newChildrens;
                for (auto oldChildEnt: children_ents) {
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
    std::unordered_map<Ent, std::unordered_map<Type, std::shared_ptr<IComponent>>> addEnts;
    std::unordered_map<Ent, std::unordered_map<Type, std::shared_ptr<IComponent>>> addComps;
    std::unordered_set<Ent> delEnts;
    std::unordered_map<Ent, std::unordered_set<Type>> delComps;
    std::unordered_map<Ent, std::unordered_map<Type, std::shared_ptr<IComponent>>> dontDestroyes;

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
    constexpr void emplace(const Type&& type, std::unique_ptr<IResource>&& resource) noexcept {
        typeMap.emplace(std::move(type), std::move(resource));
    }

    [[nodiscard]] constexpr auto get(this auto& self, const Type& type) noexcept -> auto& {
        return self.typeMap.at(type);
    }

    constexpr void clear() noexcept {
        typeMap.clear();
    }

private:
    std::unordered_map<std::size_t, std::unique_ptr<IResource>> typeMap;
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
        lateFixedSystems.emplace_back(std::move(cond), std::move(funcs));
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

class Time final: public IResource {
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

    template <typename T, typename... Ts> requires ((IsComponentConcept<T> && (IsComponentConcept<Ts> && ...)) && (!std::is_reference_v<T> || (!std::is_reference_v<Ts> || ...)) && (!std::is_const_v<T> || (!std::is_const_v<Ts> || ...)))
    [[nodiscard("La valeur de retour d'une commande Has doit toujours etre evalue")]] auto has_components_this_frame(const Ent& ent) const noexcept -> bool {
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

    template <typename T, typename... Ts> requires ((IsComponentConcept<T> && (IsComponentConcept<Ts> && ...)) && (!std::is_reference_v<T> || (!std::is_reference_v<Ts> || ...)) && (!std::is_const_v<T> || (!std::is_const_v<Ts> || ...)))
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
        return has_components_this_frame<T, Ts...>(ent);
    }

private:
    template <typename T>
    [[nodiscard]] auto internalGet(const Ent& ent) noexcept -> std::optional<std::reference_wrapper<T>> {
        if (reg.has(ent, {typeid(T).hash_code()})) {
            auto& component = reg.get(ent, typeid(T).hash_code());
            return static_cast<T&>(component);
        }
        return std::nullopt;
    }

    template <typename T>
    [[nodiscard]] auto internalGet(const Ent& ent) const noexcept -> std::optional<const std::reference_wrapper<T>> {
        if (reg.has(ent, {typeid(T).hash_code()})) {
            auto& component = reg.get(ent, typeid(T).hash_code());
            return static_cast<const T&>(component);
        }
        return std::nullopt;
    }

    template <typename T>
    [[nodiscard]] auto internalGetThisFrame(const Ent& ent) noexcept -> std::optional<std::reference_wrapper<T>> {
        if (auto addEntsIt = lateUpgrade.addEnts.find(ent); addEntsIt != lateUpgrade.addEnts.end()) {
            if (auto addEntsTypeIt = addEntsIt->second.find(typeid(T).hash_code()); addEntsTypeIt != addEntsIt->second.end()) {
                return *static_cast<T*>(addEntsTypeIt->second.get());
            }
        }
        if (auto addCompsIt = lateUpgrade.addComps.find(ent); addCompsIt != lateUpgrade.addComps.end()) {
            if (auto addCompsTypeIt = addCompsIt->second.find(typeid(T).hash_code()); addCompsTypeIt != addCompsIt->second.end()) {
                return *static_cast<T*>(addCompsTypeIt->second.get());
            }
        }
        if (reg.has(ent, {typeid(T).hash_code()})) {
            auto& component = reg.get(ent, typeid(T).hash_code());
            return *static_cast<T*>(component.get());
        }
        return std::nullopt;
    }

    template <typename T>
    [[nodiscard]] auto internalGetThisFrame(const Ent& ent) const noexcept -> std::optional<const std::reference_wrapper<T>> {
        if (auto addEntsIt = lateUpgrade.addEnts.find(ent); addEntsIt != lateUpgrade.addEnts.end()) {
            if (auto addEntsTypeIt = addEntsIt->second.find(typeid(T).hash_code()); addEntsTypeIt != addEntsIt->second.end()) {
                return static_cast<const T&>(addEntsTypeIt->second);
            }
        }
        if (auto addCompsIt = lateUpgrade.addComps.find(ent); addCompsIt != lateUpgrade.addComps.end()) {
            if (auto addCompsTypeIt = addCompsIt->second.find(typeid(T).hash_code()); addCompsTypeIt != addCompsIt->second.end()) {
                return static_cast<const T&>(addCompsTypeIt->second);
            }
        }
        if (reg.has(ent, {typeid(T).hash_code()})) {
            auto& component = reg.get(ent, typeid(T).hash_code());
            return static_cast<const T&>(component);
        }
        return std::nullopt;
    }

public:
    template <typename T, typename... Ts> requires (IsComponentConcept<T> && IsNotEmptyConcept<T> && IsNotSameConcept<T, Ts...> && !std::is_reference_v<T>)
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

    template <typename T, typename... Ts> requires (IsComponentConcept<T> && IsNotEmptyConcept<T> && IsNotSameConcept<T, Ts...> && !std::is_reference_v<T>)
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

    template <typename... Ts> requires ((sizeof...(Ts) > 0) && (IsResourceConcept<Ts> && ...) && (!std::is_reference_v<Ts> || ...))
    [[nodiscard("La valeur de retour d'une commande Resource doit toujours etre recupere")]] auto resource(this auto& self) noexcept -> std::tuple<Ts&...> {
        return std::forward_as_tuple(*static_cast<Ts*>(self.res.get(typeid(Ts).hash_code()).get())...);
    }

    [[nodiscard]] auto getDestroyedEnts() const noexcept -> const std::unordered_set<Ent>& {
        return lateUpgrade.delEnts;
    }

    [[nodiscard]] auto getAddedEnts() const noexcept -> const std::unordered_map<Ent, std::unordered_map<Type, std::shared_ptr<IComponent>>>& {
        return lateUpgrade.addEnts;
    }

    [[nodiscard]] auto getAddedComps() const noexcept -> const std::unordered_map<Ent, std::unordered_map<Type, std::shared_ptr<IComponent>>>& {
        return lateUpgrade.addComps;
    }

    [[nodiscard]] auto getDestroyedComps() const noexcept -> const std::unordered_map<Ent, std::unordered_set<Type>>& {
        return lateUpgrade.delComps;
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        (IsComponentConcept<Comps> && ...) &&
        (IsComponentConcept<Filters> && ...) &&
        (IsComponentConcept<Excludes> && ...) &&
        !((std::is_reference_v<Comps>) || ...) &&
        !((std::is_const_v<Filters> || std::is_reference_v<Filters>) || ...) &&
        !((std::is_const_v<Excludes> || std::is_reference_v<Excludes>) || ...)
    )
    [[nodiscard]] auto query(With<Filters...> = {}, Without<Excludes...> = {}) noexcept -> const View<Comps...> {
        return reg.view<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(IsInactive).hash_code(), typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        (IsComponentConcept<Comps> && ...) &&
        (IsComponentConcept<Filters> && ...) &&
        (IsComponentConcept<Excludes> && ...) &&
        !((std::is_reference_v<Comps>) || ...) &&
        !((std::is_const_v<Filters> || std::is_reference_v<Filters>) || ...) &&
        !((std::is_const_v<Excludes> || std::is_reference_v<Excludes>) || ...)
    )
    [[nodiscard]] auto query(Without<Excludes...>, With<Filters...> = {}) noexcept -> const View<Comps...> {
        return reg.view<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(IsInactive).hash_code(), typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        (IsComponentConcept<Comps> && ...) &&
        (IsComponentConcept<Filters> && ...) &&
        (IsComponentConcept<Excludes> && ...) &&
        !((std::is_reference_v<Comps>) || ...) &&
        !((std::is_const_v<Filters> || std::is_reference_v<Filters>) || ...) &&
        !((std::is_const_v<Excludes> || std::is_reference_v<Excludes>) || ...)
    )
    [[nodiscard]] auto query(With<Filters...>, Without<Excludes...>, WithInactive) noexcept -> const View<Comps...> {
        return reg.view<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        (IsComponentConcept<Comps> && ...) &&
        (IsComponentConcept<Filters> && ...) &&
        (IsComponentConcept<Excludes> && ...) &&
        !((std::is_reference_v<Comps>) || ...) &&
        !((std::is_const_v<Filters> || std::is_reference_v<Filters>) || ...) &&
        !((std::is_const_v<Excludes> || std::is_reference_v<Excludes>) || ...)
    )
    [[nodiscard]] auto query(Without<Excludes...>, With<Filters...>, WithInactive) noexcept -> const View<Comps...> {
        return reg.view<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        (IsComponentConcept<Comps> && ...) &&
        (IsComponentConcept<Filters> && ...) &&
        (IsComponentConcept<Excludes> && ...) &&
        !((std::is_reference_v<Comps>) || ...) &&
        !((std::is_const_v<Filters> || std::is_reference_v<Filters>) || ...) &&
        !((std::is_const_v<Excludes> || std::is_reference_v<Excludes>) || ...)
    )
    [[nodiscard]] auto query(With<Filters...>, WithInactive, Without<Excludes...> = {}) noexcept -> const View<Comps...> {
        return reg.view<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        (IsComponentConcept<Comps> && ...) &&
        (IsComponentConcept<Filters> && ...) &&
        (IsComponentConcept<Excludes> && ...) &&
        !((std::is_reference_v<Comps>) || ...) &&
        !((std::is_const_v<Filters> || std::is_reference_v<Filters>) || ...) &&
        !((std::is_const_v<Excludes> || std::is_reference_v<Excludes>) || ...)
    )
    [[nodiscard]] auto query(Without<Excludes...>, WithInactive, With<Filters...> = {}) noexcept -> const View<Comps...> {
        return reg.view<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        (IsComponentConcept<Comps> && ...) &&
        (IsComponentConcept<Filters> && ...) &&
        (IsComponentConcept<Excludes> && ...) &&
        !((std::is_reference_v<Comps>) || ...) &&
        !((std::is_const_v<Filters> || std::is_reference_v<Filters>) || ...) &&
        !((std::is_const_v<Excludes> || std::is_reference_v<Excludes>) || ...)
    )
    [[nodiscard]] auto query(WithInactive, With<Filters...> = {}, Without<Excludes...> = {}) noexcept -> const View<Comps...> {
        return reg.view<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        (IsComponentConcept<Comps> && ...) &&
        (IsComponentConcept<Filters> && ...) &&
        (IsComponentConcept<Excludes> && ...) &&
        !((std::is_reference_v<Comps>) || ...) &&
        !((std::is_const_v<Filters> || std::is_reference_v<Filters>) || ...) &&
        !((std::is_const_v<Excludes> || std::is_reference_v<Excludes>) || ...)
    )
    [[nodiscard]] auto query(WithInactive, Without<Excludes...>, With<Filters...> = {}) noexcept -> const View<Comps...> {
        return reg.view<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(Excludes).hash_code()...});
    }

    template <typename... Comps> requires ((IsComponentConcept<Comps> && ...) && IsNotSameConcept<Comps...>)
    auto create_entity(Comps&&... comps) noexcept -> const Ent {
        return lateUpgrade.newEnt(
            reg.getEntToken(),
            {{typeid(Comps).hash_code(), (std::is_empty_v<Comps> ? nullptr : std::make_shared<Comps>(std::move(comps)))}...}
        );
    }

    template <typename Comp, typename... Comps> requires (IsComponentConcept<Comp> && IsNotSameConcept<Comps...>)
    auto add_components(const Ent& ent, Comp&& comp, Comps&&... comps) noexcept -> std::optional<std::tuple<Comp&, Comps&...>> {
        if (reg.exist(ent)) {
            lateUpgrade.add(
                reg,
                ent,
                std::initializer_list<std::tuple<const Type, const char*, std::shared_ptr<IComponent>>>{
                    {typeid(Comp).hash_code(), typeid(Comp).name(), (std::is_empty_v<Comp> ? nullptr : std::make_shared<Comp>(std::move(comp)))},
                    {typeid(Comps).hash_code(), typeid(Comps).name(), (std::is_empty_v<Comps> ? nullptr : std::make_shared<Comps>(std::move(comps)))}...
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
        world.res.emplace(typeid(Time).hash_code(), std::make_unique<Time>(0.02f));
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

    template <typename T, typename... Args> requires (IsResourceConcept<T>)
    [[nodiscard]] auto add_resource(Args&&... args) noexcept -> ZerEngine& {
        world.res.emplace(typeid(T).hash_code(), std::make_unique<T>(std::forward<Args>(args)...));
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