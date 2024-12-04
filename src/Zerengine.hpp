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

constexpr inline std::size_t ZERENGINE_VERSION_MAJOR = 24;
constexpr inline std::size_t ZERENGINE_VERSION_MINOR = 12;
constexpr inline std::size_t ZERENGINE_VERSION_PATCH = 0;

using Entity = std::size_t;
using Type = std::size_t;

template <typename... Filters>
class With final {};
template <typename... Filters>
constexpr inline const With<Filters...> with;

template <typename... Excludes>
class Without final {};
template <typename... Excludes>
constexpr inline const Without<Excludes...> without;

class WithInactive final {};
constexpr inline WithInactive with_inactive;

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

struct ArchetypeCreateWith final {};
constexpr inline const ArchetypeCreateWith archetypeCreateWith;
struct ArchetypeCreateWithout final {};
constexpr inline const ArchetypeCreateWithout archetypeCreateWithout;

class StartSystem final {};
constexpr inline const StartSystem startSystem;
class MainSystem final {};
constexpr inline const MainSystem mainSystem;
class MainFixedSystem final {};
constexpr inline const MainFixedSystem mainFixedSystem;
class MainUnscaledFixedSystem final {};
constexpr inline const MainUnscaledFixedSystem mainUnscaledFixedSystem;
class ThreadedSystem final {};
constexpr inline const ThreadedSystem threadedSystem;
class ThreadedFixedSystem final {};
constexpr inline const ThreadedFixedSystem threadedFixedSystem;
class ThreadedUnscaledFixedSystem final {};
constexpr inline const ThreadedUnscaledFixedSystem threadedUnscaledFixedSystem;
class LateSystem final {};
constexpr inline const LateSystem lateSystem;
class LateFixedSystem final {};
constexpr inline const LateFixedSystem lateFixedSystem;
class LateUnscaledFixedSystem final {};
constexpr inline const LateUnscaledFixedSystem lateUnscaledFixedSystem;
class CallbackSystem final {};
constexpr inline const CallbackSystem callback_system;
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
        static_assert(!impl_is_not_same_rec<std::remove_cv_t<std::remove_reference_t<Ts>>...>(with<>), "Impossible de requeter des types en doublons");
    }
    return true;
}();

template <typename T>
concept IsNotEmptyConcept = [] -> bool {
    static_assert((sizeof(T) > 8), "Impossible de requeter un Marker (objet de taille 0)");
    return true;
}();

template <typename T>
concept IsFinalConcept = [] -> bool {
    static_assert(std::is_final_v<T>, "Impossible d'ajouter un composant non final (class *** final {})");
    return true;
}();

template <typename T>
concept IsComponentConcept = [] -> bool {
    static_assert(std::is_class_v<T>, "Impossible d'ajouter un Composant qui ne soit pas une Classe/Struct");
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
friend class LateUpgrade;
public:
    [[nodiscard]] CompPool() noexcept = default;
    [[nodiscard]] CompPool(const Entity entity, std::shared_ptr<IComponent>&& component) noexcept:
        components({{entity, std::move(component)}}) {
    }

public:
    constexpr auto insert_entity(const Entity entity, std::shared_ptr<IComponent>&& component) noexcept -> void {
        components.emplace(entity, std::move(component));
    }

    [[nodiscard]] constexpr auto get_entity(const Entity entity) noexcept -> auto& {
        return components.at(entity);
    }

    [[nodiscard]] constexpr auto get_entity(const Entity entity) const noexcept -> auto& {
        return components.at(entity);
    }

    [[nodiscard]] constexpr auto contains_entity(const Entity entity) const noexcept -> bool {
        return components.contains(entity);
    }

    [[nodiscard]] auto remove_entity(const Entity entity) noexcept -> std::shared_ptr<IComponent> {
        auto component = components.at(entity);
        components.erase(entity);
        return component;
    }

private:
    std::unordered_map<Entity, std::shared_ptr<IComponent>> components;
};

// class IComponentPool {
// protected:
//     [[nodiscard]] constexpr IComponentPool() noexcept = default;

// public:
//     constexpr virtual ~IComponentPool() noexcept = default;

// public:
//     constexpr virtual void remove_component(const Entity entity) noexcept = 0;
//     constexpr virtual void clear() noexcept = 0;
//     [[nodiscard]] constexpr virtual bool empty() const noexcept = 0;
//     [[nodiscard]] constexpr virtual std::size_t size() const noexcept = 0;
// };

// template <typename T>
// class ComponentPool final: public IComponentPool {
// public:
//     constexpr void add_component(const Entity entity, T&& component) noexcept {
//         entity_components.emplace(entity, std::move(component));
//     }

//     constexpr void remove_component(const Entity entity) noexcept override final {
//         entity_components.erase(entity);
//     }

//     constexpr void clear() noexcept override final {
//         entity_components.clear();
//     }

//     [[nodiscard]] constexpr bool empty() const noexcept override final {
//         return entity_components.empty();
//     }

//     [[nodiscard]] constexpr std::size_t size() const noexcept override final {
//         entity_components.size();
//     }

// private:
//     std::unordered_map<Entity, T> entity_components;
// };

///////////////////////////////////////////////////////////////////////////////////

class Archetype final {
friend class Registry;
friend class LiteArchetype;
friend class LateUpgrade;
template <typename... Ts>
friend class Query;
public:
    Archetype() noexcept = default;

    Archetype(const Entity entity, std::unordered_map<Type, std::shared_ptr<IComponent>>&& components) noexcept:
        ents({entity}),
        pools(std::move(generate_pools(entity, std::move(components)))) {
    }

    Archetype(ArchetypeCreateWith, const std::unique_ptr<Archetype>& old_archetype, const Entity entity, std::pair<const Type, std::shared_ptr<IComponent>>&& component) noexcept:
        ents({entity}),
        pools(std::move(generate_pools(archetypeCreateWith, old_archetype, entity, std::move(component)))) {
    }

    Archetype(ArchetypeCreateWithout, const std::unique_ptr<Archetype>& old_archetype, const Entity entity, const Type component_type) noexcept:
        ents({entity}),
        pools(std::move(generate_pools(archetypeCreateWithout, old_archetype, entity, component_type))) {
    }

private:
    [[nodiscard]] static auto generate_pools(const Entity entity, std::unordered_map<Type, std::shared_ptr<IComponent>>&& components) noexcept -> std::unordered_map<Type, std::unique_ptr<CompPool>> {
        std::unordered_map<Type, std::unique_ptr<CompPool>> new_pools(components.size());
        for (auto&& [type, component]: components) {
            new_pools.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(type),
                std::forward_as_tuple(component ? std::make_unique<CompPool>(entity, std::move(component)) : nullptr)
            );
        }
        return new_pools;
    }

    [[nodiscard]] static auto generate_pools(ArchetypeCreateWith, const std::unique_ptr<Archetype>& old_archetype, const Entity entity, std::pair<const Type, std::shared_ptr<IComponent>>&& component) noexcept -> std::unordered_map<Type, std::unique_ptr<CompPool>> {
        std::unordered_map<Type, std::unique_ptr<CompPool>> new_pools(old_archetype->pools.size() + 1);
        for (auto&& [old_type, old_component]: old_archetype->pools) {
            new_pools.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(old_type),
                std::forward_as_tuple(old_component ? std::make_unique<CompPool>(entity, std::move(old_component->remove_entity(entity))) : nullptr)
            );
        }
        new_pools.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(component.first),
            std::forward_as_tuple(component.second ? std::make_unique<CompPool>(entity, std::move(component.second)) : nullptr)
        );
        old_archetype->destroy(entity);
        return new_pools;
    }

    [[nodiscard]] static auto generate_pools(ArchetypeCreateWithout, const std::unique_ptr<Archetype>& old_archetype, const Entity entity, const Type component_type) noexcept -> std::unordered_map<Type, std::unique_ptr<CompPool>> {
        std::unordered_map<Type, std::unique_ptr<CompPool>> new_pools(old_archetype->pools.size() - 1);
        for (auto&& [old_type, old_component]: old_archetype->pools) {
            if (old_type != component_type) {
                new_pools.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(old_type),
                    std::forward_as_tuple(old_component ? std::make_unique<CompPool>(entity, std::move(old_component->remove_entity(entity))) : nullptr)
                );
            }
        }
        old_archetype->destroy(entity);
        return new_pools;
    }

private:
    constexpr auto new_entity(const Entity entity, std::unordered_map<Type, std::shared_ptr<IComponent>>&& components) noexcept -> void {
        ents.emplace(entity);
        for (auto&& [type, component]: components) {
            if (component) {
                pools.at(type)->insert_entity(entity, std::move(component));
            }
        }
    }

    auto add_component(const Entity entity, const std::unique_ptr<Archetype>& oldArch, std::pair<const Type, std::shared_ptr<IComponent>>&& component) noexcept -> void {
        ents.emplace(entity);
        for (auto& [old_type, old_component_pool]: oldArch->pools) {
            if (old_component_pool) {
                pools.at(old_type)->insert_entity(entity, std::move(old_component_pool->remove_entity(entity)));
            }
        }
        if (component.second) {
            pools.at(component.first)->insert_entity(entity, std::move(component.second));
        }
        oldArch->destroy(entity);
    }

    auto remove_component(const Entity entity, const std::unique_ptr<Archetype>& oldArch, const Type component_type) noexcept -> void {
        ents.emplace(entity);
        for (auto& [old_type, old_component_pool]: oldArch->pools) {
            if (old_type != component_type && old_component_pool) {
                pools.at(old_type)->insert_entity(entity, std::move(old_component_pool->remove_entity(entity)));
            }
        }
        oldArch->destroy(entity);
    }

    [[nodiscard]] constexpr auto get(const Entity entity, const Type component_type) noexcept -> auto& {
        return pools.at(component_type)->get_entity(entity);
    }

    [[nodiscard]] constexpr auto get(const Entity entity, const Type component_type) const noexcept -> auto& {
        return pools.at(component_type)->get_entity(entity);
    }

    auto destroy(const Entity entity) noexcept -> void {
        ents.erase(entity);
        for (auto& [_, comp_pool]: pools) {
            if (comp_pool && comp_pool->contains_entity(entity)) {
                static_cast<void>(comp_pool->remove_entity(entity));
            }
        }
    }

    [[nodiscard]] constexpr auto empty() const noexcept -> bool {
        return ents.empty();
    }

    [[nodiscard]] constexpr auto size() const noexcept -> std::size_t {
        return ents.size();
    }

    [[nodiscard]] constexpr auto containsType(const Type type) const noexcept -> bool {
        return pools.contains(type);
    }

private:
    template <typename... Ts>
    [[nodiscard]] constexpr auto getTupleWithEnt(const Entity entity) noexcept -> std::tuple<const Entity, Ts&...> {
        return std::forward_as_tuple(entity, (*static_cast<Ts*>(get(entity, typeid(Ts).hash_code()).get()))...);
    }

private:
    [[nodiscard]] constexpr auto isTotalyCompatibleLate(const std::unordered_map<Type, std::shared_ptr<IComponent>>& components) const noexcept -> bool {
        if (components.size() != pools.size()) {
            return false;
        }
        for (const auto& [type, _]: components) {
            if (!pools.contains(type)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] auto isTotalyCompatibleLate(const std::unique_ptr<Archetype>& oldArch, const Type component_type) const noexcept -> bool {
        if (oldArch->pools.size() + 1 != pools.size() || !pools.contains(component_type)) {
            return false;
        }
        for (const auto& [old_type, _]: oldArch->pools) {
            if (!pools.contains(old_type)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] auto isTotalyCompatibleWithoutLate(const std::unique_ptr<Archetype>& oldArch, const Type component_type) const noexcept -> bool {
        if (oldArch->pools.size() - 1 != pools.size() || pools.contains(component_type)) {
            return false;
        }
        for (const auto& [old_type, _]: oldArch->pools) {
            if (!pools.contains(old_type) && old_type != component_type) {
                return false;
            }
        }
        return true;
    }

private:
    std::unordered_set<Entity> ents;
    const std::unordered_map<Type, std::unique_ptr<CompPool>> pools;
};

///////////////////////////////////////////////////////////////////////////////////

template <typename... Ts>
class Query final {
friend class Registry;
friend class LiteRegistry;
private:
    constexpr Query(std::unordered_map<std::size_t, std::reference_wrapper<Archetype>>&& newArchs) noexcept:
        archs(std::move(newArchs)) {
    }

public:
    [[nodiscard]] constexpr auto empty() const noexcept -> bool {
        if (archs.empty()) {
            return true;
        }
        for (const auto& [_, archetype]: archs) {
            if (!archetype.get().empty()) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr auto size() const noexcept -> std::size_t {
        std::size_t newSize = 0;
        for (const auto& [_, archetype]: archs) {
            newSize += archetype.get().size();
        }
        return newSize;
    }

private:
    class QueryIterator final {
    friend class Query;
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::tuple<const Entity, Ts&...>;
        using element_type = value_type;
        using pointer = value_type*;
        using reference = value_type&;
        using difference_type = std::ptrdiff_t;

    public:
        QueryIterator(const std::unordered_map<std::size_t, std::reference_wrapper<Archetype>>& newArchs, std::unordered_map<std::size_t, std::reference_wrapper<Archetype>>::const_iterator newArchsIt) noexcept:
            archsIt(newArchsIt),
            archs(newArchs) {
            if (archsIt != newArchs.end()) {
                entsIt = archsIt->second.get().ents.begin();
            }
        }

        QueryIterator(const QueryIterator&) = default;
        QueryIterator(QueryIterator&&) = default;

        auto operator=(const QueryIterator& oth) -> QueryIterator& = delete;
        auto operator=(QueryIterator&& oth) -> QueryIterator& = delete;

        ~QueryIterator() = default;

        [[nodiscard]] constexpr auto operator *() const noexcept -> value_type {
            return archsIt->second.get().template getTupleWithEnt<Ts...>((*entsIt));
        }

        constexpr auto operator ++() noexcept -> QueryIterator& {
            entsIt++;
            if (entsIt == archsIt->second.get().ents.end()) {
                archsIt++;
                if (archsIt != archs.end()) {
                    entsIt = archsIt->second.get().ents.begin();
                }
            }
            return *this;
        }

        [[nodiscard]] friend constexpr auto operator !=(const QueryIterator& a, const QueryIterator& b) noexcept -> bool {
            return a.archsIt != b.archsIt;
        }

    private:
        std::unordered_map<std::size_t, std::reference_wrapper<Archetype>>::const_iterator archsIt;
        std::unordered_set<Entity>::iterator entsIt;
        const std::unordered_map<std::size_t, std::reference_wrapper<Archetype>>& archs;
    };

public:
    [[nodiscard]] constexpr auto begin() const noexcept -> QueryIterator {
        return {archs, archs.begin()};
    }

    [[nodiscard]] constexpr auto end() const noexcept -> QueryIterator {
        return {archs, archs.end()};
    }

private:
    const std::unordered_map<std::size_t, std::reference_wrapper<Archetype>> archs;
};

///////////////////////////////////////////////////////////////////////////////////

class Registry final {
friend class World;
friend class LateUpgrade;
private:
    Registry() noexcept {
        archs.emplace(0, std::move(std::make_unique<Archetype>()));
    }

private:
    [[nodiscard]] constexpr auto getEntToken() noexcept -> Entity {
        Entity token = lastEnt++;

        if (!entTokens.empty()) {
            lastEnt--;
            token = entTokens.back();
            entTokens.pop_back();
        }

        // entArch.emplace(token, 0);

        return token;
    }

    [[nodiscard]] constexpr auto get_archetype_token() noexcept -> std::size_t {
        auto token = last_archetype_token++;

        if (!archetype_tokens.empty()) {
            last_archetype_token--;
            token = archetype_tokens.back();
            archetype_tokens.pop_back();
        }

        return token;
    }

    auto newEnt(const Entity entity, std::unordered_map<Type, std::shared_ptr<IComponent>>&& components) noexcept -> void {
        std::unordered_map<std::size_t, std::reference_wrapper<Archetype>> compatible_archetypes(archs.size());
        for (auto& [archetype_id, archetype]: archs) {
            compatible_archetypes.emplace(archetype_id, *archetype);
        }
        for (const auto& [type, _]: components) {
            filterArchsByType(type, compatible_archetypes);
        }
        for (auto& [archetype_id, archetype]: compatible_archetypes) {
            if (archetype.get().isTotalyCompatibleLate(components)) {
                archetype.get().new_entity(entity, std::move(components));
                // entArchIt->second = archetype_id;
                entArch.emplace(entity, archetype_id);
                return;
            }
        }

        const auto& [new_archetype_id, new_archetype] = *archs.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(get_archetype_token()),
            std::forward_as_tuple(std::make_unique<Archetype>(entity, std::move(components)))
        ).first;
        for (const auto& [type, _]: new_archetype->pools) {
            emplaceArchByType(type, new_archetype_id);
        }
        // entArchIt->second = new_archetype_id;
        entArch.emplace(entity, new_archetype_id);
    }

    auto add(const Entity entity, std::pair<const Type, std::shared_ptr<IComponent>>&& component) noexcept -> void {
        auto entArchIt = entArch.find(entity);
        if (entArchIt == entArch.end()) {
            std::println("ZerEngine::Registry::add - Impossible d'ajouter un composant sur une entité inexistante - Entity[{}]", entity);
            return;
        }

        auto archetypes_it = archs.find(entArchIt->second);
        if (archetypes_it == archs.end()) {
            std::println("ZerEngine::Registry::add - Impossible d'utiliser un archetype - Entity[{}]", entity);
            return;
        }

        if (archetypes_it->second->containsType(component.first)) {
            std::println("ZerEngine - Registry: Impossible d'ajouter 2 composants identiques  - Entity[{}]", entity);
            return;
        }

        const auto& old_archetype_id = archetypes_it->first;
        const auto& old_archetype = archetypes_it->second;

        std::unordered_map<std::size_t, std::reference_wrapper<Archetype>> compatible_archetypes(archs.size());
        for (auto& [archetype_id, archetype]: archs) {
            compatible_archetypes.emplace(archetype_id, *archetype);
        }
        for (const auto& [old_type, _]: old_archetype->pools) {
            filterArchsByType(old_type, compatible_archetypes);
        }
        filterArchsByType(component.first, compatible_archetypes);
        for (auto& [archetype_id, archetype]: compatible_archetypes) {
            if (archetype.get().isTotalyCompatibleLate(old_archetype, component.first)) {
                archetype.get().add_component(entity, old_archetype, std::move(component));
                emplaceArchByType(component.first, archetype_id);
                removeOldArchIfEmpty(old_archetype_id);
                entArchIt->second = archetype_id;
                return;
            }
        }

        const auto& [new_archetype_id, new_archetype] = *archs.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(get_archetype_token()),
            std::forward_as_tuple(std::make_unique<Archetype>(archetypeCreateWith, old_archetype, entity, std::move(component)))
        ).first;
        for (const auto& [type, _]: new_archetype->pools) {
            emplaceArchByType(type, new_archetype_id);
        }
        removeOldArchIfEmpty(old_archetype_id);
        entArchIt->second = new_archetype_id;
    }

    auto remove(const Entity entity, const Type component_type) noexcept -> void {
        auto entArchIt = entArch.find(entity);
        if (entArchIt == entArch.end()) {
            std::println("ZerEngine::Registry::remove - Impossible de supprimer un composant sur une entite inexistante - [{}]", entity);
            return;
        }

        auto archetypes_it = archs.find(entArchIt->second);
        if (archetypes_it == archs.end()) {
            std::println("ZerEngine::Registry::remove - Impossible de supprimer 2 composants identiques  - Entity[{}]", entity);
            return;
        }

        if (!archetypes_it->second->containsType(component_type)) {
            std::println("ZerEngine::Registry::remove - Impossible de supprimer 2 composants identiques  - Entity[{}]", entity);
            return;
        }

        const auto& old_archetype_id = archetypes_it->first;
        const auto& old_archetype = archetypes_it->second;

        std::unordered_map<std::size_t, std::reference_wrapper<Archetype>> compatible_archetypes(archs.size());
        for (auto& [archetype_id, archetype]: archs) {
            compatible_archetypes.emplace(archetype_id, *archetype);
        }
        for (const auto& [old_type, _]: old_archetype->pools) {
            if (old_type != component_type) {
                filterArchsByType(old_type, compatible_archetypes);
            }
        }
        for (auto& [archetype_id, archetype]: compatible_archetypes) {
            if (archetype.get().isTotalyCompatibleWithoutLate(old_archetype, component_type)) {
                archetype.get().remove_component(entity, old_archetype, component_type);
                removeOldArchIfEmpty(old_archetype_id);
                entArchIt->second = archetype_id;
                return;
            }
        }

        const auto& [new_archetype_id, new_archetype] = *archs.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(get_archetype_token()),
            std::forward_as_tuple(std::make_unique<Archetype>(archetypeCreateWithout, old_archetype, entity, component_type))
        ).first;
        for (const auto& [type, _]: new_archetype->pools) {
            emplaceArchByType(type, new_archetype_id);
        }
        removeOldArchIfEmpty(old_archetype_id);
        entArchIt->second = new_archetype_id;
    }

    [[nodiscard]] constexpr auto exist(const Entity entity) const noexcept -> bool {
        return entArch.contains(entity);
    }

    [[nodiscard]] auto has(const Entity entity, const std::initializer_list<Type>& types) const noexcept -> bool {
        auto entArchIt = entArch.find(entity);
        if (entArchIt == entArch.end()) {
            return false;
        }
        auto archetypes_it = archs.find(entArchIt->second);
        if (archetypes_it == archs.end()) {
            return false;
        }
        for (const auto& type: types) {
            if (!archetypes_it->second->pools.contains(type)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr auto get(const Entity entity, const Type component_type) noexcept -> auto& {
        return archs.at(entArch.at(entity))->get(entity, component_type);
    }

    [[nodiscard]] constexpr auto get(const Entity entity, const Type component_type) const noexcept -> auto& {
        return archs.at(entArch.at(entity))->get(entity, component_type);
    }

    auto destroy(const Entity entity) noexcept -> void {
        auto entArchIt = entArch.find(entity);
        if (entArchIt == entArch.end()) {
            std::println("ZerEngine::Registry::Destroy - Impossible de detruire une entitée qui n'existe pas");
            return;
        }

        auto archetypes_it = archs.find(entArchIt->second);
        if (archetypes_it == archs.end()) {
            std::println("ZerEngine::Registry::Destroy - Impossible de detruire une entitée qui n'existe pas");
            return;
        }

        const auto& old_archetype_id = archetypes_it->first;
        const auto& old_archetype = archetypes_it->second;
        old_archetype->destroy(entity);
        entTokens.push_back(entity);
        detachChildren(entity);
        removeParent(entity);
        removeOldArchIfEmpty(old_archetype_id);
        entArch.erase(entArchIt);
    }

    auto clean() noexcept -> void {
        lastEnt = 1;
        entTokens.clear();
        last_archetype_token = 1;
        archetype_tokens.clear();
        archs.clear();
        archs.emplace(0, std::make_unique<Archetype>());
        entArch.clear();
        archsByType.clear();
        parentChildrens.clear();
        childrenParent.clear();
    }

private:
    constexpr auto appendChildrenInactiveRecDown(const Entity parentEntity) noexcept -> void {
        if (auto childrenOpt = getChildren(parentEntity)) {
            for (auto childEnt: childrenOpt.value().get()) {
                if (!has(childEnt, {typeid(IsInactive).hash_code()})) {
                    add(childEnt, std::pair(typeid(IsInactive).hash_code(), std::make_shared<IsInactive>()));
                }
                appendChildrenInactiveRecDown(childEnt);
            }
        }
    }

    constexpr auto appendChildrenInactiveRecUp(const Entity parentEntity) noexcept -> void {
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

    constexpr auto appendChildrenDontDestroyOnLoadRecDown(const Entity parentEntity) noexcept -> void {
        if (auto childrenOpt = getChildren(parentEntity)) {
            for (auto childEnt: childrenOpt.value().get()) {
                if (!has(childEnt, {typeid(DontDestroyOnLoad).hash_code()})) {
                    add(childEnt, std::pair(typeid(DontDestroyOnLoad).hash_code(), std::make_shared<DontDestroyOnLoad>()));
                }
                appendChildrenDontDestroyOnLoadRecDown(childEnt);
            }
        }
    }

    constexpr auto appendChildrenDontDestroyOnLoadRecUp(const Entity parentEntity) noexcept -> void {
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

    auto appendChildren(const Entity parentEntity, const std::unordered_set<Entity>& childrenEnt) noexcept -> void {
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

    auto appendChildren(const Entity parentEntity, const std::vector<Entity>& childrenEnt) noexcept -> void {
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

    auto detachChildren(const Entity parentEntity) noexcept -> void {
        if (auto parentIt = parentChildrens.find(parentEntity); parentIt != parentChildrens.end()) {
            for (const auto childEnt: parentIt->second) {
                if (auto childrenIt = childrenParent.find(childEnt); childrenIt != childrenParent.end()) {
                    childrenParent.erase(childrenIt);
                }
            }
            parentChildrens.erase(parentIt);
        }
    }

    auto removeParent(const Entity childEntity) noexcept -> void {
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

    [[nodiscard]] constexpr auto hasChildren(const Entity parentEntity) const noexcept -> bool {
        return parentChildrens.contains(parentEntity);
    }

    [[nodiscard]] auto getChildren(const Entity parentEntity) const noexcept -> std::optional<std::reference_wrapper<const std::unordered_set<Entity>>> {
        if (auto parentIt = parentChildrens.find(parentEntity); parentIt != parentChildrens.end()) {
            return std::make_optional<std::reference_wrapper<const std::unordered_set<Entity>>>(std::reference_wrapper<const std::unordered_set<Entity>>(parentIt->second));
        }
        return std::nullopt;
    }

    [[nodiscard]] constexpr auto hasParent(const Entity childEntity) const noexcept -> bool {
        return childrenParent.contains(childEntity);
    }

    [[nodiscard]] auto getParent(const Entity childEntity) const noexcept -> std::optional<Entity> {
        if (auto childIt = childrenParent.find(childEntity); childIt != childrenParent.end()) {
            return childIt->second;
        }
        return std::nullopt;
    }

private:
    template <typename... Comps>
    [[nodiscard]] constexpr auto query(const std::initializer_list<Type>& compFilterTypes, const std::initializer_list<Type>& excludeTypes) const noexcept -> const Query<Comps...> {
        std::unordered_map<std::size_t, std::reference_wrapper<Archetype>> internal_archetypes;
        if (compFilterTypes.size() > 0) {
            query_with_components(internal_archetypes, compFilterTypes);
        } else {
            internal_archetypes.reserve(archs.size());
            for (const auto& [archetype_id, archetype]: archs) {
                internal_archetypes.emplace(archetype_id, *archetype);
            }
        }
        if (excludeTypes.size() > 0) {
            query_without_components(internal_archetypes, excludeTypes);
        }
        return {std::move(internal_archetypes)};
    }

private:
    constexpr auto query_with_components(std::unordered_map<std::size_t, std::reference_wrapper<Archetype>>& internal_archetypes, const std::initializer_list<Type>& compTypes) const noexcept -> void {
        std::size_t i = 0;
        for (const auto& component_type: compTypes) {
            if (i == 0) {
                if (auto archsByTypeIt = archsByType.find(component_type); archsByTypeIt != archsByType.end()) {
                    for (const auto& archetype_id: archsByTypeIt->second) {
                        internal_archetypes.emplace(archetype_id, *archs.at(archetype_id));
                    }
                } else {
                    internal_archetypes.clear();
                    return;
                }
                i++;
            } else {
                if (auto archsByTypeIt = archsByType.find(component_type); archsByTypeIt != archsByType.end()) {
                    const auto& other_archetypes_id_set = archsByTypeIt->second;
                    std::vector<std::size_t> to_erase;
                    for (const auto& [archetype_id, _]: internal_archetypes) {
                        if (!other_archetypes_id_set.contains(archetype_id)) {
                            to_erase.emplace_back(archetype_id);
                        }
                    }
                    for (const auto& archetype_id: to_erase) {
                        internal_archetypes.erase(archetype_id);
                    }
                } else {
                    internal_archetypes.clear();
                    return;
                }
            }
        }
    }

    constexpr auto query_without_components(std::unordered_map<std::size_t, std::reference_wrapper<Archetype>>& internal_archetypes, const std::initializer_list<Type>& component_types) const noexcept -> void {
        for (const auto& component_type: component_types) {
            if (auto archsByTypeIt = archsByType.find(component_type); archsByTypeIt != archsByType.end()) {
                for (const auto& archetype_id: archsByTypeIt->second) {
                    if (internal_archetypes.contains(archetype_id)) {
                        internal_archetypes.erase(archetype_id);
                    }
                }
            }
        }
    }

    auto filterArchsByType(const Type component_type, std::unordered_map<std::size_t, std::reference_wrapper<Archetype>>& compatible_archetypes) noexcept -> void {
        std::unordered_map<std::size_t, std::reference_wrapper<Archetype>> newArchs;
        if (auto archsByTypeIt = archsByType.find(component_type); archsByTypeIt != archsByType.end()) {
            for (const auto& archetype_id: archsByTypeIt->second) {
                if (compatible_archetypes.contains(archetype_id)) {
                    newArchs.emplace(archetype_id, compatible_archetypes.at(archetype_id));
                }
            }
        }
        compatible_archetypes = newArchs;
    }

private:
    auto emplaceArchByType(const Type component_type, const std::size_t archetype_id) noexcept -> void {
        if (auto archsByTypeIt = archsByType.find(component_type); archsByTypeIt != archsByType.end()) {
            archsByTypeIt->second.emplace(archetype_id);
        } else {
            archsByType.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(component_type),
                std::forward_as_tuple(std::initializer_list<std::size_t>{archetype_id})
            );
        }
    }

    auto removeOldArchIfEmpty(const std::size_t old_archetype_id) noexcept -> void {
        const auto& old_archetype = archs.at(old_archetype_id);
        if (old_archetype->empty() && old_archetype_id != 0) {
            for (const auto& [old_type, _]: old_archetype->pools) {
                archsByType.at(old_type).erase(old_archetype_id);
            }
            archs.erase(old_archetype_id);
        }
    }

private:
    Entity lastEnt = 1;
    std::vector<Entity> entTokens;
    std::size_t last_archetype_token = 1;
    std::vector<std::size_t> archetype_tokens;
    std::unordered_map<std::size_t, std::unique_ptr<Archetype>> archs;
    std::unordered_map<Entity, std::size_t> entArch;
    std::unordered_map<Type, std::unordered_set<std::size_t>> archsByType;
    std::unordered_map<Entity, std::unordered_set<Entity>> parentChildrens;
    std::unordered_map<Entity, Entity> childrenParent;

    // std::unordered_map<Type, std::unique_ptr<IComponentPool>> components;
};

///////////////////////////////////////////////////////////////////////////////////

class World;

class LateUpgrade final {
friend class World;
private:
    LateUpgrade() = default;

private:
    auto newEnt(const Entity ent, std::initializer_list<std::pair<const Type, std::shared_ptr<IComponent>>>&& newList) noexcept -> Entity {
        const std::unique_lock<std::mutex> lock(mtx);
        addEnts.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(ent),
            std::forward_as_tuple(std::move(newList))
        );
        return ent;
    }

    void add(const Registry& reg, const Entity ent, std::initializer_list<std::tuple<const Type, const char*, std::shared_ptr<IComponent>>>&& newList) noexcept {
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

    void remove(const Entity ent, const Type type) noexcept {
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

    void appendChildren(const Entity parentEnt, const std::vector<Entity>& childrenEnt) {
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

    void set_active(const Entity entity) {
        setActiveEnts.emplace(entity);
    }

    void set_inactive(const Entity entity) {
        setInactiveEnts.emplace(entity);
    }

    void add_dont_destroy_on_load(const Entity entity) {
        addDontDestroyOnLoadEnts.emplace(entity);
    }

    void destroyChildRec(Registry& registry, const Entity parent_entity) noexcept {
        if (registry.exist(parent_entity)) {
            if (auto childrenOpt = registry.getChildren(parent_entity)) {
                const std::unordered_set<Entity> copyChildrenSet = childrenOpt.value().get();
                for (const auto childEnt: copyChildrenSet) {
                    destroyChildRec(registry, childEnt);
                }
            }
            registry.destroy(parent_entity);
        }
    }

    void destroy(const Entity entity) noexcept {
        const std::unique_lock<std::mutex> lock(mtx);
        delEnts.emplace(entity);
    }

    void load_scene(std::function<void(SceneSystem, World&)>&& newScene) noexcept {
        needClean = true;
        newSceneFunc = std::move(newScene);
    }

private:
    void setActiveRec(Registry& registry, const Entity entity) {
        if (registry.has(entity, {typeid(IsInactive).hash_code()})) {
            registry.remove(entity, typeid(IsInactive).hash_code());
            if (auto opt_children = registry.getChildren(entity)) {
                for (auto child_entity: opt_children.value().get()) {
                    setActiveRec(registry, child_entity);
                }
            }
        }
    }

    void setInactiveRec(Registry& registry, const Entity entity) {
        if (!registry.has(entity, {typeid(IsInactive).hash_code()})) {
            registry.add(entity, std::pair(typeid(IsInactive).hash_code(), std::make_shared<IsInactive>()));
            if (auto opt_children = registry.getChildren(entity)) {
                for (auto childEnt: opt_children.value().get()) {
                    setInactiveRec(registry, childEnt);
                }
            }
        }
    }

    void addDontDetroyOnLoadRec(Registry& registry, const Entity entity) {
        if (!registry.has(entity, {typeid(DontDestroyOnLoad).hash_code()})) {
            registry.add(entity, std::pair(typeid(DontDestroyOnLoad).hash_code(), std::make_shared<DontDestroyOnLoad>()));
            if (auto opt_children = registry.getChildren(entity)) {
                for (auto child_entity: opt_children.value().get()) {
                    addDontDetroyOnLoadRec(registry, child_entity);
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

        for (const Entity ent: delEnts) {
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
            std::unordered_map<Entity, std::unordered_set<Entity>> dontDestroyesHierarchies;
            for (auto [dontDestroyEnt]: reg.query({typeid(DontDestroyOnLoad).hash_code()}, {})) {
                const std::unique_ptr<Archetype>& arch = reg.archs.at(reg.entArch.at(dontDestroyEnt));
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

            std::unordered_map<Entity, Entity> oldToNewEnts;
            for (auto&& [entity, components]: dontDestroyes) {
                auto newEntId = reg.getEntToken();
                reg.newEnt(newEntId, std::move(components));
                oldToNewEnts.emplace(entity, newEntId);
            }
            dontDestroyes.clear();

            for (const auto& [parent_ent, children_ents]: dontDestroyesHierarchies) {
                auto newEntId = oldToNewEnts.at(parent_ent);
                std::unordered_set<Entity> newChildrens;
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
    std::unordered_map<Entity, std::unordered_map<Type, std::shared_ptr<IComponent>>> addEnts;
    std::unordered_map<Entity, std::unordered_map<Type, std::shared_ptr<IComponent>>> addComps;
    std::unordered_set<Entity> delEnts;
    std::unordered_map<Entity, std::unordered_set<Type>> delComps;
    std::unordered_map<Entity, std::unordered_map<Type, std::shared_ptr<IComponent>>> dontDestroyes;

    std::unordered_map<Entity, std::unordered_set<Entity>> addParentChildren;

    std::unordered_set<Entity> setActiveEnts;
    std::unordered_set<Entity> setInactiveEnts;

    std::unordered_set<Entity> addDontDestroyOnLoadEnts;

    bool needClean = false;
    std::function<void(SceneSystem, World&)> newSceneFunc;
};

///////////////////////////////////////////////////////////////////////////////////

class TypeMap final {
friend class World;
friend class ZerEngine;
private:
    constexpr void emplace(const Type type, std::unique_ptr<IResource>&& resource) noexcept {
        type_map.emplace(type, std::move(resource));
    }

    [[nodiscard]] constexpr auto get(const Type type) noexcept -> auto& {
        return type_map.at(type);
    }

    [[nodiscard]] constexpr auto get(const Type type) const noexcept -> auto& {
        return type_map.at(type);
    }

    constexpr void clear() noexcept {
        type_map.clear();
    }

private:
    std::unordered_map<Type, std::unique_ptr<IResource>> type_map;
};

///////////////////////////////////////////////////////////////////////////////////

class ThreadedFixedSet final {
friend class Sys;
public:
    [[nodiscard]] constexpr ThreadedFixedSet(std::initializer_list<ThreadedFixedSet>&& new_sub_sets) noexcept:
        condition(nullptr),
        tasks(),
        subSets(std::move(new_sub_sets)) {
    }

    [[nodiscard]] constexpr ThreadedFixedSet(bool(*const new_condtion)(World&), std::initializer_list<ThreadedFixedSet>&& new_sub_sets) noexcept:
        condition(new_condtion),
        tasks(),
        subSets(std::move(new_sub_sets)) {
    }

    [[nodiscard]] constexpr ThreadedFixedSet(std::initializer_list<void(*)(ThreadedFixedSystem, World&)>&& new_tasks, std::initializer_list<ThreadedFixedSet>&& new_sub_sets = {}) noexcept:
        condition(nullptr),
        tasks(std::move(new_tasks)),
        subSets(std::move(new_sub_sets)) {
    }

    [[nodiscard]] constexpr ThreadedFixedSet(bool(*const new_condtion)(World&), std::initializer_list<void(*)(ThreadedFixedSystem, World&)>&& new_tasks, std::initializer_list<ThreadedFixedSet>&& new_sub_sets = {}) noexcept:
        condition(new_condtion),
        tasks(std::move(new_tasks)),
        subSets(std::move(new_sub_sets)) {
    }

private:
    bool(*const condition)(World&);
    const std::vector<void(*)(ThreadedFixedSystem, World&)> tasks;
    const std::vector<ThreadedFixedSet> subSets;
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

    constexpr void add_callback_system(void(*const callback)(CallbackSystem, World&, const Entity), const Entity entity) noexcept {
        callback_systems.emplace_back(callback, entity);
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

    void run_callbacks(World& world) {
        for (const auto& [callback, entity]: callback_systems) {
            callback({}, world, entity);
        }
        callback_systems.clear();
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
    std::vector<std::pair<void(*)(CallbackSystem, World&, const Entity), Entity>> callback_systems;
    ThreadPool threadpool;
    bool isUseMultithreading {true};
};

///////////////////////////////////////////////////////////////////////////////////

class Time final: public IResource {
friend class ZerEngine;
public:
    Time(const float& newFixedTimeStep = 0.02f) noexcept:
        t2(std::chrono::high_resolution_clock::now()),
        totalTime(std::chrono::high_resolution_clock::now()),
        fixedTimeStep(newFixedTimeStep) {
    }

private:
    constexpr void set_fixed_time_step(const float& newFixedTimeStep) noexcept {
        fixedTimeStep = newFixedTimeStep;
    }

    void update() noexcept {
        auto t1 = std::chrono::high_resolution_clock::now();
        dt = std::chrono::duration_cast<std::chrono::duration<float>>(t1 - t2).count();
        t2 = t1;
        timeScale = newTimeScale;
        #ifdef DISPLAY_FPS
            frame_counter();
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

    [[nodiscard]] constexpr auto unscaled_delta() const noexcept -> float {
        return dt;
    }

    [[nodiscard]] constexpr auto unscaled_fixed_delta() const noexcept -> float {
        return fixedTimeStep;
    }

    [[nodiscard]] constexpr auto fixed_delta() const noexcept -> float {
        return fixedTimeStep * timeScale;
    }

    [[nodiscard]] constexpr auto is_time_step() const noexcept -> bool {
        return isTimeStepFrame;
    }

    [[nodiscard]] constexpr auto get_nb_fixed_steps() const noexcept -> unsigned int {
        return nbFixedSteps;
    }

    [[nodiscard]] constexpr auto get_time_scale() const noexcept -> float {
        return newTimeScale;
    }

    constexpr void set_time_scale(const float& newScale) noexcept {
        newTimeScale = newScale;
    }

private:
    #ifdef DISPLAY_FPS
        void frame_counter() noexcept {
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
    [[nodiscard("La valeur de retour d'une commande Exist doit toujours etre evalue")]] auto is_entity_exists(const Entity entity) const noexcept -> bool {
        return reg.exist(entity) || lateUpgrade.addEnts.contains(entity);
    }

    template <typename T, typename... Ts> requires ((IsComponentConcept<T> && (IsComponentConcept<Ts> && ...)) && (!std::is_const_v<T> || (!std::is_const_v<Ts> || ...)))
    [[nodiscard("La valeur de retour d'une commande Has doit toujours etre evalue")]] auto has_components_this_frame(const Entity entity) const noexcept -> bool {
        if (!is_entity_exists(entity)) {
            return false;
        }
        if (reg.has(entity, {typeid(T).hash_code()})) {
            if constexpr (sizeof...(Ts) > 0) {
                return has_components<Ts...>(entity);
            }
            return true;
        }
        if (auto addEntsIt = lateUpgrade.addEnts.find(entity); addEntsIt != lateUpgrade.addEnts.end() && addEntsIt->second.contains(typeid(T).hash_code())) {
            if constexpr (sizeof...(Ts) > 0) {
                return has_components<Ts...>(entity);
            }
            return true;
        }
        if (auto addCompsIt = lateUpgrade.addComps.find(entity); addCompsIt != lateUpgrade.addComps.end() && addCompsIt->second.contains(typeid(T).hash_code())) {
            if constexpr (sizeof...(Ts) > 0) {
                return has_components<Ts...>(entity);
            }
            return true;
        }
        return false;
    }

    template <typename... Ts> requires (sizeof...(Ts) > 0 && ((IsComponentConcept<Ts> && !std::is_const_v<Ts>) && ...))
    [[nodiscard("La valeur de retour d'une commande Has doit toujours etre evalue")]] auto has_components(const Entity entity) const noexcept -> bool {
        // if (!reg.exist(ent)) {
        //     return false;
        // } else if (reg.has(ent, {typeid(T).hash_code()})) {
        //     if constexpr (sizeof...(Ts) > 0) {
        //         return has<Ts...>(ent);
        //     }
        //     return true;
        // }
        // return false;
        return has_components_this_frame<Ts...>(entity);
    }

private:
    template <typename T>
    [[nodiscard]] auto internal_get_components(const Entity entity) noexcept -> std::optional<std::reference_wrapper<T>> {
        if (reg.has(entity, {typeid(T).hash_code()})) {
            auto& component = reg.get(entity, typeid(T).hash_code());
            return static_cast<T&>(component);
        }
        return std::nullopt;
    }

    template <typename T>
    [[nodiscard]] auto internal_get_components(const Entity entity) const noexcept -> std::optional<const std::reference_wrapper<T>> {
        if (reg.has(entity, {typeid(T).hash_code()})) {
            auto& component = reg.get(entity, typeid(T).hash_code());
            return static_cast<const T&>(component);
        }
        return std::nullopt;
    }

    template <typename T>
    [[nodiscard]] auto internal_get_components_this_frame(const Entity entity) noexcept -> std::optional<std::reference_wrapper<T>> {
        if (auto addEntsIt = lateUpgrade.addEnts.find(entity); addEntsIt != lateUpgrade.addEnts.end()) {
            if (auto addEntsTypeIt = addEntsIt->second.find(typeid(T).hash_code()); addEntsTypeIt != addEntsIt->second.end()) {
                return *static_cast<T*>(addEntsTypeIt->second.get());
            }
        }
        if (auto addCompsIt = lateUpgrade.addComps.find(entity); addCompsIt != lateUpgrade.addComps.end()) {
            if (auto addCompsTypeIt = addCompsIt->second.find(typeid(T).hash_code()); addCompsTypeIt != addCompsIt->second.end()) {
                return *static_cast<T*>(addCompsTypeIt->second.get());
            }
        }
        if (reg.has(entity, {typeid(T).hash_code()})) {
            auto& component = reg.get(entity, typeid(T).hash_code());
            return *static_cast<T*>(component.get());
        }
        return std::nullopt;
    }

    template <typename T>
    [[nodiscard]] auto internal_get_components_this_frame(const Entity entity) const noexcept -> std::optional<const std::reference_wrapper<T>> {
        if (auto addEntsIt = lateUpgrade.addEnts.find(entity); addEntsIt != lateUpgrade.addEnts.end()) {
            if (auto addEntsTypeIt = addEntsIt->second.find(typeid(T).hash_code()); addEntsTypeIt != addEntsIt->second.end()) {
                return static_cast<const T&>(addEntsTypeIt->second);
            }
        }
        if (auto addCompsIt = lateUpgrade.addComps.find(entity); addCompsIt != lateUpgrade.addComps.end()) {
            if (auto addCompsTypeIt = addCompsIt->second.find(typeid(T).hash_code()); addCompsTypeIt != addCompsIt->second.end()) {
                return static_cast<const T&>(addCompsTypeIt->second);
            }
        }
        if (reg.has(entity, {typeid(T).hash_code()})) {
            auto& component = reg.get(entity, typeid(T).hash_code());
            return static_cast<const T&>(component);
        }
        return std::nullopt;
    }

public:
    template <typename T, typename... Ts> requires (IsComponentConcept<T> && IsNotEmptyConcept<T> && IsNotSameConcept<T, Ts...>)
    [[nodiscard("La valeur de retour d'une commande Get doit toujours etre recupere")]] auto get_components_this_frame(const Entity entity) noexcept -> std::optional<std::tuple<T&, Ts&...>> {
        if (auto opt = internal_get_components_this_frame<T>(entity)) {
            if constexpr (sizeof...(Ts) > 0) {
                if (auto othOpt = get_components_this_frame<Ts...>(entity)) {
                    return std::tuple_cat(std::forward_as_tuple(opt.value().get()), othOpt.value());
                }
            } else {
                return std::forward_as_tuple(opt.value().get());
            }
        }
        return std::nullopt;
    }

    template <typename... Ts> requires (sizeof...(Ts) > 0 && ((IsComponentConcept<Ts> && IsNotEmptyConcept<Ts>) && ...) && IsNotSameConcept<Ts...>)
    [[nodiscard("La valeur de retour d'une commande Get doit toujours etre recupere")]] auto get_components(const Entity entity) noexcept -> std::optional<std::tuple<Ts&...>> {
        // if (auto opt = internal_get_components<T>(ent)) {
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
        return get_components_this_frame<Ts...>(entity);
    }

    [[nodiscard("La valeur de retour d'une commande HasParent doit toujours etre evaluer")]] auto has_parent(const Entity child_entity) const noexcept -> bool {
        return reg.hasParent(child_entity);
    }

    [[nodiscard("La valeur de retour d'une commande GetParent doit toujours etre recupere")]] auto get_parent(const Entity child_entity) const noexcept -> std::optional<Entity> {
        return reg.getParent(child_entity);
    }

    [[nodiscard("La valeur de retour d'une commande HasChildren doit toujours etre evaluer")]] auto has_children(const Entity parent_entity) const noexcept -> bool {
        return reg.hasChildren(parent_entity);
    }

    [[nodiscard("La valeur de retour d'une commande GetChildren doit toujours etre recupere")]] auto get_children(const Entity parent_entity) const noexcept -> std::optional<std::reference_wrapper<const std::unordered_set<Entity>>> {
        return reg.getChildren(parent_entity);
    }

    constexpr auto append_children(const Entity parent_entity, const std::vector<Entity>& children_entity) noexcept -> Entity {
        lateUpgrade.appendChildren(parent_entity, children_entity);
        return parent_entity;
    }

    template <typename... Ts> requires ((sizeof...(Ts) > 0) && (IsResourceConcept<Ts> && ...))
    [[nodiscard("La valeur de retour d'une commande Resource doit toujours etre recupere")]] auto resource() noexcept -> std::tuple<Ts&...> {
        return std::forward_as_tuple(*static_cast<Ts*>(res.get(typeid(Ts).hash_code()).get())...);
    }

    template <typename... Ts> requires ((sizeof...(Ts) > 0) && (IsResourceConcept<Ts> && ...))
    [[nodiscard("La valeur de retour d'une commande Resource doit toujours etre recupere")]] auto resource() const noexcept -> std::tuple<Ts&...> {
        return std::forward_as_tuple(*static_cast<Ts*>(res.get(typeid(Ts).hash_code()).get())...);
    }

    [[nodiscard]] auto get_destroyed_ents() const noexcept -> const std::unordered_set<Entity>& {
        return lateUpgrade.delEnts;
    }

    [[nodiscard]] auto get_added_ents() const noexcept -> const std::unordered_map<Entity, std::unordered_map<Type, std::shared_ptr<IComponent>>>& {
        return lateUpgrade.addEnts;
    }

    [[nodiscard]] auto get_added_components() const noexcept -> const std::unordered_map<Entity, std::unordered_map<Type, std::shared_ptr<IComponent>>>& {
        return lateUpgrade.addComps;
    }

    [[nodiscard]] auto get_destroyed_components() const noexcept -> const std::unordered_map<Entity, std::unordered_set<Type>>& {
        return lateUpgrade.delComps;
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        (IsComponentConcept<Comps> && ...) &&
        (IsComponentConcept<Filters> && ...) &&
        (IsComponentConcept<Excludes> && ...) &&
        !(std::is_const_v<Filters> || ...) &&
        !(std::is_const_v<Excludes> || ...)
    )
    [[nodiscard]] auto query(With<Filters...> = {}, Without<Excludes...> = {}) noexcept -> const Query<Comps...> {
        return reg.query<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(IsInactive).hash_code(), typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        (IsComponentConcept<Comps> && ...) &&
        (IsComponentConcept<Filters> && ...) &&
        (IsComponentConcept<Excludes> && ...) &&
        !(std::is_const_v<Filters> || ...) &&
        !(std::is_const_v<Excludes> || ...)
    )
    [[nodiscard]] auto query(Without<Excludes...>, With<Filters...> = {}) noexcept -> const Query<Comps...> {
        return reg.query<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(IsInactive).hash_code(), typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        (IsComponentConcept<Comps> && ...) &&
        (IsComponentConcept<Filters> && ...) &&
        (IsComponentConcept<Excludes> && ...) &&
        !(std::is_const_v<Filters> || ...) &&
        !(std::is_const_v<Excludes> || ...)
    )
    [[nodiscard]] auto query(With<Filters...>, Without<Excludes...>, WithInactive) noexcept -> const Query<Comps...> {
        return reg.query<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        (IsComponentConcept<Comps> && ...) &&
        (IsComponentConcept<Filters> && ...) &&
        (IsComponentConcept<Excludes> && ...) &&
        !(std::is_const_v<Filters> || ...) &&
        !(std::is_const_v<Excludes> || ...)
    )
    [[nodiscard]] auto query(Without<Excludes...>, With<Filters...>, WithInactive) noexcept -> const Query<Comps...> {
        return reg.query<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        (IsComponentConcept<Comps> && ...) &&
        (IsComponentConcept<Filters> && ...) &&
        (IsComponentConcept<Excludes> && ...) &&
        !(std::is_const_v<Filters> || ...) &&
        !(std::is_const_v<Excludes> || ...)
    )
    [[nodiscard]] auto query(With<Filters...>, WithInactive, Without<Excludes...> = {}) noexcept -> const Query<Comps...> {
        return reg.query<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        (IsComponentConcept<Comps> && ...) &&
        (IsComponentConcept<Filters> && ...) &&
        (IsComponentConcept<Excludes> && ...) &&
        !(std::is_const_v<Filters> || ...) &&
        !(std::is_const_v<Excludes> || ...)
    )
    [[nodiscard]] auto query(Without<Excludes...>, WithInactive, With<Filters...> = {}) noexcept -> const Query<Comps...> {
        return reg.query<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        (IsComponentConcept<Comps> && ...) &&
        (IsComponentConcept<Filters> && ...) &&
        (IsComponentConcept<Excludes> && ...) &&
        !(std::is_const_v<Filters> || ...) &&
        !(std::is_const_v<Excludes> || ...)
    )
    [[nodiscard]] auto query(WithInactive, With<Filters...> = {}, Without<Excludes...> = {}) noexcept -> const Query<Comps...> {
        return reg.query<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(Excludes).hash_code()...});
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    requires (
        (IsNotEmptyConcept<Comps> && ...) &&
        IsNotSameConcept<Comps..., Filters..., Excludes...> &&
        (IsComponentConcept<Comps> && ...) &&
        (IsComponentConcept<Filters> && ...) &&
        (IsComponentConcept<Excludes> && ...) &&
        !(std::is_const_v<Filters> || ...) &&
        !(std::is_const_v<Excludes> || ...)
    )
    [[nodiscard]] auto query(WithInactive, Without<Excludes...>, With<Filters...> = {}) noexcept -> const Query<Comps...> {
        return reg.query<Comps...>({typeid(Comps).hash_code()..., typeid(Filters).hash_code()...}, {typeid(Excludes).hash_code()...});
    }

    template <typename... Comps> requires ((IsComponentConcept<Comps> && ...) && IsNotSameConcept<Comps...>)
    auto create_entity(Comps&&... comps) noexcept -> Entity {
        return lateUpgrade.newEnt(
            reg.getEntToken(),
            {{typeid(Comps).hash_code(), (std::is_empty_v<Comps> ? nullptr : std::make_shared<Comps>(std::move(comps)))}...}
        );
    }

    template <typename... Comps> requires (sizeof...(Comps) > 0 && (IsComponentConcept<Comps> && ...) && IsNotSameConcept<Comps...>)
    auto add_components(const Entity entity, Comps&&... comps) noexcept -> std::optional<std::tuple<Comps&...>> {
        if (is_entity_exists(entity)) {
            lateUpgrade.add(
                reg,
                entity,
                {{typeid(Comps).hash_code(), typeid(Comps).name(), (std::is_empty_v<Comps> ? nullptr : std::make_shared<Comps>(std::move(comps)))}...}
            );
        } else {
            (std::println("World::add_component(): Impossible d'ajouter sur une entitée qui n'existe pas [type: {}]", typeid(Comps).name()), ...);
            return std::nullopt;
        }

        return std::forward_as_tuple(internal_get_components_this_frame<Comps>(entity).value()...);
    }

    template <typename T, typename... Ts>
    void remove_components(const Entity entity) noexcept {
        if (has_components_this_frame<T>(entity)) {
            lateUpgrade.remove(entity, typeid(T).hash_code());
        } else {
            std::println("World::remove_component(): Impossible de supprimer un composant qui n'existe pas - {}", typeid(T).name());
        }

        if constexpr (sizeof...(Ts) > 0) {
            remove_components<Ts...>(entity);
        }
    }

    void delete_entity(const Entity entity) noexcept {
        if (is_entity_exists(entity)) {
            lateUpgrade.destroy(entity);
        } else {
            std::println("World::delete_entity(): Impossible de supprimer une entitée qui n'existe pas");
        }
    }

    constexpr void use_callback(void(*const callback)(CallbackSystem, World&, const Entity), const Entity entity) {
        if (callback) {
            sys.add_callback_system(callback, entity);
        }
    }

    void set_active(const Entity ent) noexcept {
        lateUpgrade.set_active(ent);
    }

    void set_inactive(const Entity ent) noexcept {
        lateUpgrade.set_inactive(ent);
    }

    [[nodiscard]] constexpr auto get_total_entities() const noexcept -> std::size_t {
        return reg.entArch.size();
    }

    [[nodiscard]] constexpr auto get_total_archetypes() const noexcept -> std::size_t {
        return reg.archs.size();
    }

    void add_dont_destroy_on_load(const Entity ent) noexcept {
        lateUpgrade.add_dont_destroy_on_load(ent);
    }

    void load_scene(std::function<void(SceneSystem, World&)>&& newScene) noexcept {
        lateUpgrade.load_scene(std::move(newScene));
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
        time.set_fixed_time_step(newFixedTimeStep);
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

            if (time.is_time_step()) {
                if (time.timeScale != 0) {
                    for (unsigned int i = 0; i < time.get_nb_fixed_steps(); i++) {
                        world.upgrade();
                        world.sys.runFixed(world);
                    }
                }
                for (unsigned int i = 0; i < time.get_nb_fixed_steps(); i++) {
                    world.upgrade();
                    world.sys.runUnscaledFixed(world);
                }
            }

            world.upgrade();
            world.sys.run_callbacks(world);

            world.upgrade();
            world.sys.runLate(world);
            world.upgrade();
        }
    }

private:
    World world;
};

///////////////////////////////////////////////////////////////////////////////////
