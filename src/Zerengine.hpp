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
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <print>
#include <ranges>
#include <set>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

constexpr inline std::size_t ZERENGINE_VERSION_MAJOR = 25;
constexpr inline std::size_t ZERENGINE_VERSION_MINOR = 3;
constexpr inline std::size_t ZERENGINE_VERSION_PATCH = 1;

using Entity = std::size_t;
using Type = std::size_t;

template <typename... Filters>
struct [[nodiscard]] With final {};
template <typename... Filters>
constexpr inline const With<Filters...> with;

template <typename... Excludes>
struct [[nodiscard]] Without final {};
template <typename... Excludes>
constexpr inline const Without<Excludes...> without;

struct [[nodiscard]] WithInactive final {};
constexpr inline WithInactive with_inactive;

class [[nodiscard]] IComponent {
protected:
    constexpr IComponent() noexcept = default;

public:
    constexpr ~IComponent() noexcept = default;
};

class [[nodiscard]] WithCascadingInsert {
protected:
    constexpr WithCascadingInsert() noexcept = default;

public:
    constexpr ~WithCascadingInsert() noexcept = default;
};

class [[nodiscard]] WithCascadingRemove {
protected:
    constexpr WithCascadingRemove() noexcept = default;

public:
    constexpr ~WithCascadingRemove() noexcept = default;
};

enum class CascadeMode: uint8_t {
    INSERT,
    REMOVE,
    INSERT_AND_REMOVE,
};

struct [[nodiscard]] IsInactive final: public IComponent, public WithCascadingInsert {};
struct [[nodiscard]] DontDestroyOnLoad final: public IComponent, public WithCascadingInsert {};

struct [[nodiscard]] Parent final: public IComponent {
friend class Registry;
public:
    constexpr Parent(const Entity new_parent_entity) noexcept:
        parent_entity(new_parent_entity) {
    }

    [[nodiscard]] constexpr operator Entity() const noexcept {
        return parent_entity;
    }

private:
    Entity parent_entity;
};

struct [[nodiscard]] Children final: public IComponent {
friend class Registry;
public:
    constexpr Children(const std::unordered_set<Entity>& new_children_entities) noexcept:
        children_entities(new_children_entities) {
    }

    [[nodiscard]] constexpr auto begin() const noexcept -> std::unordered_set<Entity>::const_iterator {
        return children_entities.begin();
    }

    [[nodiscard]] constexpr auto end() const noexcept -> std::unordered_set<Entity>::const_iterator {
        return children_entities.end();
    }

private:
    std::unordered_set<Entity> children_entities;
};

class [[nodiscard]] IResource {
protected:
    constexpr IResource() noexcept = default;

public:
    constexpr virtual ~IResource() noexcept = default;
};

struct [[nodiscard]] StartSystem final {};
constexpr inline const StartSystem start_system;
struct [[nodiscard]] MainSystem final {};
constexpr inline const MainSystem main_system;
struct [[nodiscard]] MainFixedSystem final {};
constexpr inline const MainFixedSystem main_fixed_system;
struct [[nodiscard]] MainUnscaledFixedSystem final {};
constexpr inline const MainUnscaledFixedSystem main_unscaled_fixed_system;
struct [[nodiscard]] ThreadedSystem final {};
constexpr inline const ThreadedSystem threaded_system;
struct [[nodiscard]] ThreadedFixedSystem final {};
constexpr inline const ThreadedFixedSystem threaded_fixed_system;
struct [[nodiscard]] ThreadedUnscaledFixedSystem final {};
constexpr inline const ThreadedUnscaledFixedSystem threaded_unscaled_fixed_system;
struct [[nodiscard]] LateSystem final {};
constexpr inline const LateSystem late_system;
struct [[nodiscard]] LateFixedSystem final {};
constexpr inline const LateFixedSystem late_fixed_system;
struct [[nodiscard]] LateUnscaledFixedSystem final {};
constexpr inline const LateUnscaledFixedSystem late_unscaled_fixed_system;
struct [[nodiscard]] CallbackSystem final {};
constexpr inline const CallbackSystem callback_system;
struct [[nodiscard]] SceneSystem final {};

struct [[nodiscard]] OnAddComponentHook final {};
constexpr inline const OnAddComponentHook on_add_component_hook;
struct [[nodiscard]] OnCreateEntityHook final {};
constexpr inline const OnCreateEntityHook on_create_entity_hook;
struct [[nodiscard]] OnRemoveComponentHook final {};
constexpr inline const OnRemoveComponentHook on_remove_component_hook;
struct [[nodiscard]] OnDeleteEntityHook final {};
constexpr inline const OnDeleteEntityHook on_delete_entity_hook;

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
    static_assert(std::is_class_v<T>, "Impossible d'ajouter un Composant qui ne soit pas une Classe/Struct");
    static_assert(std::derived_from<T, IComponent>, "Impossible d'ajouter un Composant qui n'implemente pas IComponent");
    static_assert(std::is_final_v<T>, "Impossible d'ajouter un Composant qui ne soit pas Final");
    static_assert(std::copy_constructible<T>, "Impossible d'ajouter un Composant qui ne soit pas Copiable");
    return true;
}();

template <typename T>
concept IsResourceConcept = [] -> bool {
    static_assert(std::is_class_v<T>, "Impossible d'ajouter une Ressource qui ne soit pas une Classe");
    static_assert(std::derived_from<T, IResource>, "Impossible d'ajouter une Ressource qui n'implemente pas IResource");
    static_assert(std::is_final_v<T>, "Impossible d'ajouter une Ressource qui ne soit pas Final");
    static_assert(std::copy_constructible<T>, "Impossible d'ajouter une Ressource qui ne soit pas Copiable");
    return true;
}();

///////////////////////////////////////////////////////////////////////////////////

class Registry;

enum class RegistryMessageType: uint8_t {
    CREATE_ENTITY,
    ADD_COMPONENT,
    REMOVE_COMPONENT,
    DELETE_ENTITY,
    APPEND_CHILDREN,
    SET_ACTIVE,
    SET_INACTIVE,
    ADD_DONT_DESTROY_ON_LOAD,
};

class [[nodiscard]] RegistryMessage final {
public:
    RegistryMessage(RegistryMessageType new_message_type, void(*const new_callback)(Registry&, const Entity, std::pair<Type, std::unique_ptr<IComponent>>&&, const std::vector<Type>&, const std::vector<Entity>&), const Entity new_entity, std::pair<Type, std::unique_ptr<IComponent>>&& new_component) noexcept:
        callback(new_callback),
        entity(new_entity),
        component(std::move(new_component)),
        message_type(new_message_type) {
    }

    RegistryMessage(RegistryMessageType new_message_type, void(*const new_callback)(Registry&, const Entity, std::pair<Type, std::unique_ptr<IComponent>>&&, const std::vector<Type>&, const std::vector<Entity>&), const Entity new_entity, std::vector<Type>&& new_types) noexcept:
        callback(new_callback),
        entity(new_entity),
        component_types(std::move(new_types)),
        message_type(new_message_type) {
    }

    RegistryMessage(RegistryMessageType new_message_type, void(*const new_callback)(Registry&, const Entity, std::pair<Type, std::unique_ptr<IComponent>>&&, const std::vector<Type>&, const std::vector<Entity>&), const Entity new_entity) noexcept:
        callback(new_callback),
        entity(new_entity),
        message_type(new_message_type) {
    }

    RegistryMessage(RegistryMessageType new_message_type, void(*const new_callback)(Registry&, const Entity, std::pair<Type, std::unique_ptr<IComponent>>&&, const std::vector<Type>&, const std::vector<Entity>&), const Entity new_entity, const std::vector<Entity>& new_children_entities) noexcept:
        callback(new_callback),
        entity(new_entity),
        children_entities(new_children_entities),
        message_type(new_message_type) {
    }

public:
    void(*const callback)(Registry&, const Entity, std::pair<Type, std::unique_ptr<IComponent>>&&, const std::vector<Type>&, const std::vector<Entity>&);
    const Entity entity;
    std::pair<Type, std::unique_ptr<IComponent>> component;
    std::vector<Type> component_types;
    std::vector<Entity> children_entities;
    RegistryMessageType message_type;
};

///////////////////////////////////////////////////////////////////////////////////

class [[nodiscard]] Archetype final {
friend class Registry;
friend class LiteArchetype;
friend class LateUpgrade;
template <typename... Ts>
friend class Query;
public:
    Archetype() noexcept {
        nb_archetypes++;
    }

    Archetype(const std::shared_ptr<Archetype>& old_archetype, const Type new_type) noexcept:
        types(std::move(generate_pools(old_archetype, new_type))),
        previous_archetype(old_archetype) {
        nb_archetypes++;
    }

    ~Archetype() {
        nb_archetypes--;
    }

private:
    [[nodiscard]] static auto generate_pools(const std::shared_ptr<Archetype>& old_archetype, const Type new_type) noexcept -> std::set<Type> {
        auto new_types = old_archetype->types;
        new_types.emplace(new_type);
        return new_types;
    }

private:
    constexpr void emplace_entity(const Entity entity) noexcept {
        entity_components.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(entity),
            std::forward_as_tuple()
        );
    }

    constexpr void move_entity_with(const std::shared_ptr<Archetype>& old_archetype, const Entity entity, std::pair<Type, std::unique_ptr<IComponent>>&& new_component) noexcept {
        entity_components.insert(std::move(old_archetype->entity_components.extract(entity)));
        if (new_component.second != nullptr) {
            entity_components.at(entity).emplace(std::move(new_component));
        }
    }

    constexpr void move_entity_without(const std::shared_ptr<Archetype>& old_archetype, const Entity entity, const Type new_type) noexcept {
        entity_components.insert(std::move(old_archetype->entity_components.extract(entity)));
        entity_components.at(entity).erase(new_type);
    }

    constexpr void delete_entity(const Entity entity) noexcept {
        entity_components.erase(entity);
    }

    [[nodiscard]] constexpr auto get_component(const Entity entity, const Type type) noexcept -> const std::unique_ptr<IComponent>& {
        return entity_components.at(entity).at(type);
    }

public:
    static inline std::size_t nb_archetypes = 0;
    const std::set<Type> types;
    std::unordered_map<Entity, std::unordered_map<Type, std::unique_ptr<IComponent>>> entity_components;
    std::weak_ptr<Archetype> previous_archetype;
    std::map<Type, std::shared_ptr<Archetype>> next_archetypes;
};

///////////////////////////////////////////////////////////////////////////////////

template <typename... Ts>
class [[nodiscard]] Query final {
friend class Registry;
friend class LiteRegistry;
private:
    constexpr Query(const std::unordered_set<std::shared_ptr<Archetype>>& newArchs) noexcept:
        archs(newArchs) {
    }

public:
    [[nodiscard]] constexpr auto empty() const noexcept -> bool {
        return archs.empty();
    }

    [[nodiscard]] constexpr auto size() const noexcept -> std::size_t {
        std::size_t new_size = 0;
        for (const auto& archetype: archs) {
            new_size += archetype->entity_components.size();
        }
        return new_size;
    }

private:
    class [[nodiscard]] QueryIterator final {
    friend class Query;
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::tuple<const Entity, Ts&...>;
        using element_type = value_type;
        using pointer = value_type*;
        using reference = value_type&;
        using difference_type = std::ptrdiff_t;

    public:
        QueryIterator(const std::unordered_set<std::shared_ptr<Archetype>>& newArchs, std::unordered_set<std::shared_ptr<Archetype>>::const_iterator newArchsIt) noexcept:
            archsIt(newArchsIt),
            archs(newArchs) {
            if (archsIt != newArchs.end()) {
                if (!(*archsIt)->entity_components.empty()) {
                    entsIt = (*archsIt)->entity_components.begin();
                }
            }
        }

        QueryIterator(const QueryIterator&) = default;
        QueryIterator(QueryIterator&&) = default;

        auto operator=(const QueryIterator& oth) -> QueryIterator& = delete;
        auto operator=(QueryIterator&& oth) -> QueryIterator& = delete;

        ~QueryIterator() = default;

        [[nodiscard]] constexpr auto operator *() const noexcept -> value_type {
            return std::tuple_cat(std::forward_as_tuple(entsIt->first), std::forward_as_tuple(*static_cast<Ts*>((*archsIt)->get_component(entsIt->first, typeid(Ts).hash_code()).get())...));
        }

        constexpr auto operator ++() noexcept -> QueryIterator& {
            entsIt++;
            if (entsIt == (*archsIt)->entity_components.end()) {
                archsIt++;
                if (archsIt != archs.end()) {
                    entsIt = (*archsIt)->entity_components.begin();
                }
            }
            return *this;
        }

        [[nodiscard]] friend constexpr auto operator !=(const QueryIterator& a, const QueryIterator& b) noexcept -> bool {
            return a.archsIt != b.archsIt;
        }

    private:
        std::unordered_set<std::shared_ptr<Archetype>>::const_iterator archsIt;
        std::unordered_map<Entity, std::unordered_map<Type, std::unique_ptr<IComponent>>>::iterator entsIt;
        const std::unordered_set<std::shared_ptr<Archetype>>& archs;
    };

public:
    [[nodiscard]] constexpr auto begin() const noexcept -> QueryIterator {
        return {archs, archs.begin()};
    }

    [[nodiscard]] constexpr auto end() const noexcept -> QueryIterator {
        return {archs, archs.end()};
    }

private:
    const std::unordered_set<std::shared_ptr<Archetype>> archs;
};

///////////////////////////////////////////////////////////////////////////////////

class [[nodiscard]] Registry final {
friend class World;
friend class LateUpgrade;
private:
    [[nodiscard]] constexpr auto get_entity_token() noexcept -> Entity {
        auto token = last_entity_token++;

        if (!entity_tokens.empty()) {
            last_entity_token--;
            token = entity_tokens.back();
            entity_tokens.pop_back();
            return token;
        }

        while (entArch.contains(token)) {
            token = last_entity_token++;
        }

        return token;
    }

public:
    constexpr void create_entity(const Entity entity) noexcept {
        if (entArch.contains(entity)) {
            std::cerr << "Registry::create_entity(): Impossible d'ajouter deux fois la meme entité: Entity[" << entity << "]" << std::endl;
            return;
        }

        entArch.emplace(entity, archetype_root);
        archetype_root->emplace_entity(entity);
    }

    constexpr void add_components(const Entity entity, std::pair<Type, std::unique_ptr<IComponent>>&& new_component) noexcept {
        auto entArchIt = entArch.find(entity);
        if (entArchIt == entArch.end()) {
            std::cerr << "Registry::add_components(): Impossible d'ajouter un composant sur une entite inexistante: Entity[" << entity << "]" << std::endl;
            return;
        }

        if (entArchIt->second->types.contains(new_component.first)) {
            std::cerr << "Registry::add_components(): Impossible d'ajouter deux fois le meme composant sur une entite: Entity[" << entity << "]" << std::endl;
            return;
        }

        auto old_archetype = entArchIt->second;

        if (auto next_archetype_it = old_archetype->next_archetypes.find(new_component.first); next_archetype_it != old_archetype->next_archetypes.end()) {
            entArchIt->second = next_archetype_it->second;
        } else {
            if (!old_archetype->types.empty() && *std::prev(old_archetype->types.end()) >= new_component.first) {
                if (*std::prev(old_archetype->types.end()) > new_component.first) {
                    auto ordered_types = old_archetype->types;
                    ordered_types.emplace(new_component.first);
                    create_branch(ordered_types, entity);
                }
            } else {
                entArchIt->second = old_archetype->next_archetypes.emplace(
                    new_component.first,
                    std::make_shared<Archetype>(old_archetype, new_component.first)
                ).first->second;
            }
        }
        entArchIt->second->move_entity_with(old_archetype, entity, std::move(new_component));

        graph_readjustement(old_archetype);
    }

    constexpr void remove_components(const Entity entity, const std::vector<Type>& new_types) noexcept {
        auto entArchIt = entArch.find(entity);
        if (entArchIt == entArch.end()) {
            std::cerr << "Registry::remove_components(): Impossible de supprimer un composant sur une entite inexistante: Entity[" << entity << "]" << std::endl;
            return;
        }

        for (auto new_type: new_types) {
            if (!entArchIt->second->types.contains(new_type)) {
                std::cerr << "Registry::remove_components(): Impossible de supprimer un composant inexistant sur une entite: Entity[" << entity << "]" << std::endl;
                return;
            }

            auto old_archetype = entArchIt->second;

            if (*std::prev(old_archetype->types.end()) == new_type) {
                entArchIt->second = old_archetype->previous_archetype.lock();
            } else {
                auto ordered_types = old_archetype->types;
                ordered_types.erase(new_type);
                create_branch(ordered_types, entity);
            }
            entArchIt->second->move_entity_without(old_archetype, entity, new_type);

            graph_readjustement(old_archetype);
        }
    }

    constexpr void delete_entity(const Entity entity) noexcept {
        if (!entArch.contains(entity)) {
            std::cerr << "Registry::delete_entity(): Impossible de supprimer une entite inexistante: Entity[" << entity << "]" << std::endl;
            return;
        }

        detach_children(entity);
        remove_parent(entity);

        auto old_archetype = entArch.find(entity)->second;

        old_archetype->delete_entity(entity);
        entity_tokens.push_back(entity);
        entArch.erase(entity);
        graph_readjustement(old_archetype);
    }

    [[nodiscard]] constexpr auto is_entity_exist(const Entity entity) const noexcept -> bool {
        return entArch.contains(entity);
    }

    [[nodiscard]] auto has_components(const Entity entity, const std::initializer_list<Type>& types) const noexcept -> bool {
        auto entArchIt = entArch.find(entity);
        if (entArchIt == entArch.end()) {
            // std::cerr << "Registry::has_component(): L'entite n'existe pas/plus [" << entity << "]" << std::endl;
            return false;
        }
        if (!entArchIt->second) {
            std::cerr << "Registry::has_component(): le noeud a expirer ?!?" << std::endl;
            return false;
        }
        for (const auto& type: types) {
            if (!entArchIt->second->types.contains(type)) {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr auto get(const Entity entity, const Type type) noexcept -> std::optional<std::reference_wrapper<std::unique_ptr<IComponent>>> {
        if (auto entArchIt = entArch.find(entity); entArchIt != entArch.end()) {
            if (auto entity_components_it = entArchIt->second->entity_components.find(entity); entity_components_it != entArchIt->second->entity_components.end()) {
                if (auto components_it = entity_components_it->second.find(type); components_it != entity_components_it->second.end()) {
                    return components_it->second;
                }
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] constexpr auto get_all_components_types(const Entity entity) const noexcept -> const std::set<Type>& {
        return entArch.at(entity)->types;
    }

    void clear_without(const std::unordered_set<Entity>& without_entities) noexcept {
        std::vector<Entity> remove_entities;
        for (const auto entity: std::views::keys(entArch)) {
            if (!without_entities.contains(entity)) {
                remove_entities.emplace_back(entity);
            }
        }
        for (const auto entity: remove_entities) {
            delete_entity(entity);
        }
    }

public:
    template <typename ComponentType>
    void append_children_rec_down(const Entity parent_entity) noexcept {
        if (auto opt_children = get_children(parent_entity)) {
            for (auto child_entity: opt_children.value()) {
                if (is_entity_exist(child_entity)) {
                    if (!has_components(child_entity, {typeid(ComponentType).hash_code()})) {
                        add_components(child_entity, {typeid(ComponentType).hash_code(), std::make_unique<ComponentType>()});
                    }
                    append_children_rec_down<ComponentType>(child_entity);
                }
            }
        }
    }

    template <typename ComponentType>
    void append_children_rec_up(const Entity parent_entity) noexcept {
        if (has_components(parent_entity, {typeid(ComponentType).hash_code()})) {
            if (auto opt_children = get_children(parent_entity)) {
                for (auto child_entity: opt_children.value()) {
                    if (is_entity_exist(child_entity)) {
                        if (!has_components(child_entity, {typeid(ComponentType).hash_code()})) {
                            add_components(child_entity, {typeid(ComponentType).hash_code(), std::make_unique<ComponentType>()});
                        }
                        append_children_rec_down<ComponentType>(child_entity);
                    }
                }
            }
            return;
        }

        if (auto opt_parent = get_parent(parent_entity)) {
            append_children_rec_up<ComponentType>(opt_parent.value());
        }
    }

public:
    void append_children(const Entity parent_entity, const std::vector<Entity>& children_entities) noexcept {
        if (!entArch.contains(parent_entity)) {
            std::cerr << "Registry::append_children(): Impossible d'ajouter sur une entite inexistante: Entity[" << parent_entity << "]" << std::endl;
            return;
        }

        std::unordered_set<Entity> new_children_entities;

        for (const auto child_entity: children_entities) {
            if (is_entity_exist(child_entity)) {
                if (has_components(child_entity, {typeid(Parent).hash_code()})) {
                    std::println("Children: Tu ne peux pas avoir deux parents Billy[{}]", child_entity);
                } else if (parent_entity == child_entity) {
                    std::println("Children: Impossible d'etre son propre pere");
                } else {
                    add_components(child_entity, {typeid(Parent).hash_code(), std::make_unique<Parent>(parent_entity)});
                    new_children_entities.emplace(child_entity);
                }
            } else {
                std::cerr << "Registry::append_children(): Impossible d'ajouter une entite enfant qui n'existe pas: Entity[" << child_entity << "]" << std::endl;
            }
        }

        if (!new_children_entities.empty()) {
            if (auto opt_children = get(parent_entity, typeid(Children).hash_code())) {
                auto& children = static_cast<Children&>(*opt_children.value().get());
                children.children_entities.insert(new_children_entities.begin(), new_children_entities.end());
            } else {
                add_components(parent_entity, {typeid(Children).hash_code(), std::make_unique<Children>(new_children_entities)});
            }
        }
    }

    void detach_children(const Entity parent_entity) noexcept {
        if (auto opt_children = get(parent_entity, typeid(Children).hash_code())) {
            for (const auto child_entity: static_cast<Children&>(*opt_children.value().get()).children_entities) {
                remove_components(child_entity, {typeid(Parent).hash_code()});
            }
            remove_components(parent_entity, {typeid(Children).hash_code()});
        }
    }

    void remove_parent(const Entity children_entity) noexcept {
        if (auto opt_parent = get(children_entity, typeid(Parent).hash_code())) {
            auto& parent = static_cast<Parent&>(*opt_parent.value().get());
            if (auto opt_children = get(parent.parent_entity, typeid(Children).hash_code())) {
                auto& children = static_cast<Children&>(*opt_children.value().get());
                children.children_entities.erase(children_entity);
                if (children.children_entities.empty()) {
                    remove_components(parent.parent_entity, {typeid(Children).hash_code()});
                }
            }
            remove_components(children_entity, {typeid(Parent).hash_code()});
        }
    }

    [[nodiscard]] auto get_children(const Entity parent_entity) noexcept -> std::optional<std::unordered_set<Entity>> {
        if (auto opt_children = get(parent_entity, typeid(Children).hash_code())) {
            auto& children = static_cast<Children&>(*opt_children.value().get());
            return children.children_entities;
        }
        return std::nullopt;
    }

    [[nodiscard]] auto get_parent(const Entity children_entity) noexcept -> std::optional<Entity> {
        if (auto opt_parent = get(children_entity, typeid(Parent).hash_code())) {
            auto& parent = static_cast<Parent&>(*opt_parent.value().get());
            return parent.parent_entity;
        }
        return std::nullopt;
    }

private:
    template <typename... Comps>
    [[nodiscard]] constexpr auto query(const std::initializer_list<Type>& filters, const std::initializer_list<Type>& excludes) noexcept -> const Query<Comps...> {
        std::unordered_set<std::shared_ptr<Archetype>> internal_archetypes;

        std::map<Type, bool> ordered_types;
        for (const auto type: filters) {
            ordered_types.emplace(type, false);
        }
        for (const auto type: excludes) {
            ordered_types.insert_or_assign(type, true);
        }

        if (filters.size() == 0) {
            if (!archetype_root->entity_components.empty()) {
                internal_archetypes.emplace(archetype_root);
            }
        }

        query_rec(ordered_types, filters.size(), 1, ordered_types.begin(), archetype_root, 0, internal_archetypes);

        return Query<Comps...>(internal_archetypes);
    }

private:
    constexpr void query_rec(const std::map<Type, bool>& ordered_types, const std::size_t nb_types, const std::size_t current_nb_types, std::map<Type, bool>::iterator current_type_it, const std::shared_ptr<Archetype>& current_archetype, const Type filter, std::unordered_set<std::shared_ptr<Archetype>>& internal_archetypes) const noexcept {
        for (const auto& [next_type, next_archetype]: current_archetype->next_archetypes) {
            if (next_type < filter) {
                continue;
            }

            if (current_type_it != ordered_types.end()) {
                if (!current_type_it->second) {
                    if (next_type > current_type_it->first) {
                        break;
                    } else if (next_type < current_type_it->first) {
                        query_rec(ordered_types, nb_types, current_nb_types, current_type_it, next_archetype, 0, internal_archetypes);
                        continue;
                    } else {
                        if (current_nb_types >= nb_types && !next_archetype->entity_components.empty()) {
                            internal_archetypes.emplace(next_archetype);
                        }
                        query_rec(ordered_types, nb_types, current_nb_types + 1, std::next(current_type_it), next_archetype, 0, internal_archetypes);
                    }
                } else {
                    if (next_type < current_type_it->first) {
                        if (current_nb_types > nb_types && !next_archetype->entity_components.empty()) {
                            internal_archetypes.emplace(next_archetype);
                        }
                        query_rec(ordered_types, nb_types, current_nb_types, current_type_it, next_archetype, 0, internal_archetypes);
                        continue;
                    } else if (next_type > current_type_it->first) {
                        query_rec(ordered_types, nb_types, current_nb_types, std::next(current_type_it), current_archetype, current_type_it->first, internal_archetypes);
                        break;
                    } else {
                        current_type_it++;
                        continue;
                    }
                }
            } else {
                if (current_nb_types >= nb_types && !next_archetype->entity_components.empty()) {
                    internal_archetypes.emplace(next_archetype);
                }
                query_rec(ordered_types, nb_types, current_nb_types, current_type_it, next_archetype, 0, internal_archetypes);
            }
        }
    }

private:
    constexpr void create_branch(const std::set<Type>& ordered_types, const Entity entity) noexcept {
        auto current_archetype = archetype_root;
        for (const auto type: ordered_types) {
            if (auto next_archetypes_it = current_archetype->next_archetypes.find(type); next_archetypes_it != current_archetype->next_archetypes.end()) {
                current_archetype = next_archetypes_it->second;
            } else {
                current_archetype = current_archetype->next_archetypes.emplace(
                    type,
                    std::make_shared<Archetype>(current_archetype, type)
                ).first->second;
            }
        }
        entArch.at(entity) = current_archetype;
    }

    constexpr void graph_readjustement(const std::shared_ptr<Archetype>& old_archetype) noexcept {
        auto remove_old_rec = old_archetype;
        while (!remove_old_rec->previous_archetype.expired() && remove_old_rec->entity_components.empty() && remove_old_rec->next_archetypes.empty()) {
            remove_old_rec
                ->previous_archetype.lock()
                ->next_archetypes.erase(
                    *std::prev(remove_old_rec->types.end())
                );
            remove_old_rec = remove_old_rec->previous_archetype.lock();
        }
    }

private:
    Entity last_entity_token = 1;
    std::vector<Entity> entity_tokens;
    std::unordered_map<Entity, std::shared_ptr<Archetype>> entArch;
    std::shared_ptr<Archetype> archetype_root = std::make_shared<Archetype>();
};

///////////////////////////////////////////////////////////////////////////////////

static void registry_message_callback_create_entity(Registry& registry, const Entity entity, std::pair<Type, std::unique_ptr<IComponent>>&&, const std::vector<Type>&, const std::vector<Entity>&) {
    registry.create_entity(entity);
}

static void registry_message_callback_add_components(Registry& registry, const Entity entity, std::pair<Type, std::unique_ptr<IComponent>>&& components, const std::vector<Type>&, const std::vector<Entity>&) {
    registry.add_components(entity, std::move(components));
}

static void registry_message_callback_remove_components(Registry& registry, const Entity entity, std::pair<Type, std::unique_ptr<IComponent>>&&, const std::vector<Type>& new_types, const std::vector<Entity>&) {
    registry.remove_components(entity, new_types);
}

static void registry_message_callback_append_children(Registry& registry, const Entity entity, std::pair<Type, std::unique_ptr<IComponent>>&&, const std::vector<Type>&, const std::vector<Entity>& children_entities) {
    registry.append_children(entity, children_entities);
    registry.append_children_rec_up<IsInactive>(entity);
    registry.append_children_rec_up<DontDestroyOnLoad>(entity);
}

static void setInactiveRec(Registry& registry, const Entity entity) {
    if (registry.is_entity_exist(entity)) {
        if (!registry.has_components(entity, {typeid(IsInactive).hash_code()})) {
            registry.add_components(entity, {typeid(IsInactive).hash_code(), std::make_unique<IsInactive>()});
            if (auto opt_children = registry.get_children(entity)) {
                for (auto childEnt: opt_children.value()) {
                    setInactiveRec(registry, childEnt);
                }
            }
        }
    }
}

static void registry_message_callback_set_inactive(Registry& registry, const Entity entity, std::pair<Type, std::unique_ptr<IComponent>>&&, const std::vector<Type>&, const std::vector<Entity>&) {
    setInactiveRec(registry, entity);
}

static void setActiveRec(Registry& registry, const Entity entity) {
    if (registry.is_entity_exist(entity)) {
        if (registry.has_components(entity, {typeid(IsInactive).hash_code()})) {
            registry.remove_components(entity, {typeid(IsInactive).hash_code()});
            if (auto opt_children = registry.get_children(entity)) {
                for (auto child_entity: opt_children.value()) {
                    setActiveRec(registry, child_entity);
                }
            }
        }
    }
}

static void registry_message_callback_set_active(Registry& registry, const Entity entity, std::pair<Type, std::unique_ptr<IComponent>>&&, const std::vector<Type>&, const std::vector<Entity>&) {
    setActiveRec(registry, entity);
}

static void addDontDestroyOnLoadRec(Registry& registry, const Entity entity) {
    if (registry.is_entity_exist(entity)) {
        if (!registry.has_components(entity, {typeid(DontDestroyOnLoad).hash_code()})) {
            registry.add_components(entity, {typeid(DontDestroyOnLoad).hash_code(), std::make_unique<DontDestroyOnLoad>()});
            if (auto opt_children = registry.get_children(entity)) {
                for (auto child_entity: opt_children.value()) {
                    addDontDestroyOnLoadRec(registry, child_entity);
                }
            }
        }
    }
}

static void registry_message_callback_add_font_destroy_on_load(Registry& registry, const Entity entity, std::pair<Type, std::unique_ptr<IComponent>>&&, const std::vector<Type>&, const std::vector<Entity>&) {
    addDontDestroyOnLoadRec(registry, entity);
}

static void destroyChildRec(Registry& registry, const Entity parent_entity) noexcept {
    if (registry.is_entity_exist(parent_entity)) {
        if (auto childrenOpt = registry.get_children(parent_entity)) {
            for (const auto childEnt: childrenOpt.value()) {
                destroyChildRec(registry, childEnt);
            }
        }
        registry.delete_entity(parent_entity);
    }
}

static void registry_message_callback_delete_entity(Registry& registry, const Entity entity, std::pair<Type, std::unique_ptr<IComponent>>&&, const std::vector<Type>&, const std::vector<Entity>&) {
    destroyChildRec(registry, entity);
}

class World;
class Sys;

class [[nodiscard]] LateUpgrade final {
friend class World;
public:
    using RegistryMessageIndex = std::size_t;

private:
    LateUpgrade() = default;

private:
    void create_entity(const Entity entity) noexcept {
        const std::unique_lock<std::mutex> lock(mtx);
        if (add_entities.contains(entity)) {
            std::println("ZerEngine::LateUpgrade::create_entity() - Impossible de creer une deuxieme entites avec un numero deja existant");
            return;
        }
        add_entities.emplace(entity);
        registry_messages.emplace_back(
            RegistryMessageType::CREATE_ENTITY,
            registry_message_callback_create_entity,
            entity
        );
    }

    void add_components(const Registry& registry, const Entity entity, std::pair<Type, std::unique_ptr<IComponent>>&& component, const char* component_name) noexcept {
        const std::unique_lock<std::mutex> lock(mtx);
        if (registry.has_components(entity, {component.first})) {
            std::println("ZerEngine::LateUpgrade::add_components() - Impossible d'ajouter deux fois un composant sur une entite dans les registres: entity[{}], composant[{}]", entity, component_name);
            return;
        }
        if (delEnts.contains(entity)) {
            std::println("ZerEngine::LateUpgrade::add_components() - Impossible d'ajouter un composant sur une entite qui vient d'etre supprimer: entity[{}], composant[{}]", entity, component_name);
            return;
        }
        if (auto addCompsIt = addComps.find(entity); addCompsIt != addComps.end()) {
            if (addCompsIt->second.contains(component.first)) {
                std::println("ZerEngine::LateUpgrade::add_components() - Impossible d'ajouter deux fois un composant sur une entite dans le late upgrade: entity[{}], composant[{}]", entity, component_name);
                return;
            }
            addCompsIt->second.emplace(component.first, registry_messages.size());
        } else {
            addComps.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(entity),
                std::forward_as_tuple(std::unordered_map<Type, RegistryMessageIndex>({{component.first, registry_messages.size()}}))
            );
        }
        registry_messages.emplace_back(
            RegistryMessageType::ADD_COMPONENT,
            registry_message_callback_add_components,
            entity,
            std::move(std::pair<Type, std::unique_ptr<IComponent>>{std::move(component)})
        );
    }

    void remove_components(const Registry& registry, const Entity entity, const std::vector<std::pair<const char*, Type>>& components) noexcept {
        const std::unique_lock<std::mutex> lock(mtx);
        for (const auto& [component_name, type]: components) {
            if (!registry.has_components(entity, {type})) {
                std::println("ZerEngine::LateUpgrade::remove_components() - Impossible de supprimer deux fois un composant sur une entite dans les registres: entity[{}], composant[{}]", entity, component_name);
                return;
            }
            if (delEnts.contains(entity)) {
                std::println("ZerEngine::LateUpgrade::remove_components() - Impossible de supprimer un composant sur une entite qui vient d'etre supprimer: entity[{}], composant[{}]", entity, component_name);
                return;
            }
            if (auto delCompsIt = delComps.find(entity); delCompsIt != delComps.end()) {
                if (delCompsIt->second.contains(type)) {
                    std::println("ZerEngine::LateUpgrade::remove_components() - Impossible de supprimer deux fois un composant sur une entite dans le late upgrade: entity[{}], composant[{}]", entity, component_name);
                    return;
                }
                delCompsIt->second.emplace(type);
            } else {
                delComps.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(entity),
                    std::forward_as_tuple(std::initializer_list<Type>{type})
                );
            }
            registry_messages.emplace_back(
                RegistryMessageType::REMOVE_COMPONENT,
                registry_message_callback_remove_components,
                entity,
                std::move(std::vector<Type>{type})
            );
        }
    }

    void delete_entity(const Registry& registry, const Entity entity) noexcept {
        const std::unique_lock<std::mutex> lock(mtx);
        if (delEnts.contains(entity)) {
            std::println("ZerEngine::LateUpgrade::delete_entity() - Impossible de supprimer deux fois la meme entite dans le late upgrade: entity[{}]", entity);
            return;
        }
        // addComps.erase(entity);
        // delComps.erase(entity);
        // setInactiveEnts.erase(entity);
        // setActiveEnts.erase(entity);
        // addDontDestroyOnLoadEnts.erase(entity);
        // addParentChildren.erase(entity);
        // for (auto& [_, children]: addParentChildren) {
        //     children.erase(entity);
        // }
        delEnts.emplace(entity);
        delComps.emplace(entity, registry.get_all_components_types(entity) | std::ranges::to<std::unordered_set<Type>>());
        registry_messages.emplace_back(
            RegistryMessageType::DELETE_ENTITY,
            registry_message_callback_delete_entity,
            entity
        );
    }

    void append_children(const Registry& registry, const Entity parent_entity, const std::vector<Entity>& children_entity) {
        const std::unique_lock<std::mutex> lock(mtx);
        auto new_children_entity = children_entity;
        if (delEnts.contains(parent_entity)) {
            std::println("ZerEngine::LateUpgrade::append_children() - Impossible de faire une hierarchie sur une entite supprime: entity[{}]", parent_entity);
            return;
        }
        std::size_t i = 0;
        for (const auto child_entity: children_entity) {
            if (delEnts.contains(child_entity)) {
                std::println("ZerEngine::LateUpgrade::append_children() - Impossible de faire une hierarchie avec une entite supprime: entity[{}]", child_entity);
                new_children_entity.erase(new_children_entity.begin() + i);
            }
            i++;
        }
        if (addParentChildren.contains(parent_entity)) {
            addParentChildren.at(parent_entity).insert(new_children_entity.begin(), new_children_entity.end());
        } else {
            addParentChildren.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(parent_entity),
                std::forward_as_tuple(std::unordered_set<Entity>(new_children_entity.begin(), new_children_entity.end()))
            );
        }
        registry_messages.emplace_back(
            RegistryMessageType::APPEND_CHILDREN,
            registry_message_callback_append_children,
            parent_entity,
            new_children_entity
        );
    }

    void set_active(const Entity entity) {
        if (setActiveEnts.contains(entity)) {
            std::cerr << "ZerEngine::LateUpgrade::set_active() - Impossible de rendre 2 fois actif une entité: entity[" << entity << "]" << std::endl;
            return;
        }
        if (delEnts.contains(entity)) {
            std::println("ZerEngine::LateUpgrade::set_active() - Impossible de mettre une entite IsActive: entity[{}]", entity);
            return;
        }
        setActiveEnts.emplace(entity);
        registry_messages.emplace_back(
            RegistryMessageType::SET_ACTIVE,
            registry_message_callback_set_active,
            entity
        );
    }

    void set_inactive(const Entity entity) {
        if (setInactiveEnts.contains(entity)) {
            std::cerr << "ZerEngine::LateUpgrade::set_inactive() - Impossible de rendre 2 fois inactif une entité: entity[" << entity << "]" << std::endl;
            return;
        }
        if (delEnts.contains(entity)) {
            std::println("ZerEngine::LateUpgrade::set_inactive() - Impossible de mettre une entite IsInactive: entity[{}]", entity);
            return;
        }
        setInactiveEnts.emplace(entity);
        registry_messages.emplace_back(
            RegistryMessageType::SET_INACTIVE,
            registry_message_callback_set_inactive,
            entity
        );
    }

    void add_dont_destroy_on_load(const Entity entity) {
        if (addDontDestroyOnLoadEnts.contains(entity)) {
            std::cerr << "ZerEngine::LateUpgrade::add_dont_destroy_on_load() - Impossible de mettre dont destroy on load sur une entité: entity[" << entity << "]" << std::endl;
            return;
        }
        if (delEnts.contains(entity)) {
            std::println("ZerEngine::LateUpgrade::add_dont_destroy_on_load() - Impossible de mettre une entite DontDestroyOnLoad: entity[{}]", entity);
            return;
        }
        addDontDestroyOnLoadEnts.emplace(entity);
        registry_messages.emplace_back(
            RegistryMessageType::ADD_DONT_DESTROY_ON_LOAD,
            registry_message_callback_add_font_destroy_on_load,
            entity
        );
    }

    void load_scene(void(*const new_scene)(SceneSystem, World&)) noexcept {
        scene_messages.emplace_back(new_scene);
    }

    void load_scene_internal(World& world, Registry& registry, void(*const new_scene)(SceneSystem, World&)) noexcept {
        std::unordered_set<Entity> dont_destroy_entities;
        for (auto [dont_destroy_entity]: registry.query({typeid(DontDestroyOnLoad).hash_code()}, {})) {
            dont_destroy_entities.emplace(dont_destroy_entity);
        }
        registry.clear_without(dont_destroy_entities);
        new_scene({}, world);
    }

private:
    void upgrade(World& world, Registry& registry, Sys& sys) noexcept {
        for (auto&& [callback, entity, components, component_types, children_entities, message_type]: registry_messages) {
            switch (message_type) {
                case RegistryMessageType::REMOVE_COMPONENT:
                    for (const auto remove_component_type: delComps.at(entity)) {
                        upgrade_hook_remove_component(world, sys, entity, remove_component_type);
                    }
                    break;
                case RegistryMessageType::DELETE_ENTITY:
                    for (const auto type: delComps.at(entity)) {
                        upgrade_hook_remove_component(world, sys, entity, type);
                        upgrade_hook_delete_entity_with_component(world, sys, entity, type);
                    }
                    break;
                default: break;
            }

            callback(registry, entity, std::move(components), component_types, children_entities);

            switch (message_type) {
                case RegistryMessageType::ADD_COMPONENT:
                    if (add_entities.contains(entity)) {
                        for (const auto [add_component_type, _]: addComps.at(entity)) {
                            upgrade_hook_create_entity_with_component(world, sys, entity, add_component_type);
                        }
                    } else {
                        for (const auto [add_component_type, _]: addComps.at(entity)) {
                            upgrade_hook_add_component(world, sys, entity, add_component_type);
                        }
                    }
                    break;
                default: break;
            }
        }

        add_entities.clear();
        addComps.clear();
        delComps.clear();
        delEnts.clear();
        addParentChildren.clear();
        setInactiveEnts.clear();
        setActiveEnts.clear();
        addDontDestroyOnLoadEnts.clear();

        registry_messages.clear();

        for (const auto& new_scene: scene_messages) {
            load_scene_internal(world, registry, new_scene);
        }

        scene_messages.clear();
    }

    // Co-dependency: see after class Sys final;
    constexpr void upgrade_hook_add_component(World&, Sys&, const Entity, const Type) noexcept;
    constexpr void upgrade_hook_create_entity_with_component(World&, Sys&, const Entity, const Type) noexcept;
    constexpr void upgrade_hook_remove_component(World&, Sys&, const Entity, const Type) noexcept;
    constexpr void upgrade_hook_delete_entity_with_component(World&, Sys&, const Entity, const Type) noexcept;

private:
    std::mutex mtx;
    std::unordered_set<Entity> add_entities;
    std::unordered_map<Entity, std::unordered_map<Type, RegistryMessageIndex>> addComps;
    std::unordered_set<Entity> delEnts;
    std::unordered_map<Entity, std::unordered_set<Type>> delComps;
    std::unordered_map<Entity, std::unordered_set<Entity>> addParentChildren;
    std::unordered_set<Entity> setInactiveEnts;
    std::unordered_set<Entity> setActiveEnts;
    std::unordered_set<Entity> addDontDestroyOnLoadEnts;

    std::vector<RegistryMessage> registry_messages;

    std::vector<void(*)(SceneSystem, World&)> scene_messages;
};

///////////////////////////////////////////////////////////////////////////////////

class [[nodiscard]] TypeMap final {
friend class World;
friend class ZerEngine;
private:
    constexpr void emplace(const Type type, std::unique_ptr<IResource>&& resource) noexcept {
        type_map.emplace(type, std::move(resource));
    }

    [[nodiscard]] constexpr std::unique_ptr<IResource>& get(const Type type) noexcept {
        return type_map.at(type);
    }

    [[nodiscard]] constexpr const std::unique_ptr<IResource>& get(const Type type) const noexcept {
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

    void addFixedTasks(const std::vector<void(*)(ThreadedFixedSystem, World&)>& newTasks) noexcept {
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
    std::vector<std::vector<void(*)(ThreadedFixedSystem, World&)>> fixedTasks;
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

class [[nodiscard]] Sys final {
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

    void add_on_add_component_hooks(const Type new_type, std::initializer_list<std::function<void(OnAddComponentHook, World&, const Entity)>>&& callback) noexcept {
        if (auto on_add_component_hooks_it = on_add_component_hooks.find(new_type); on_add_component_hooks_it != on_add_component_hooks.end()) {
            on_add_component_hooks_it->second.insert(on_add_component_hooks_it->second.end(), std::move(callback));
        } else {
            on_add_component_hooks.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(new_type),
                std::forward_as_tuple(std::move(callback))
            );
        }
    }

    void add_on_create_entity_hooks(const Type new_type, std::initializer_list<std::function<void(OnCreateEntityHook, World&, const Entity)>>&& callback) noexcept {
        if (auto on_create_entity_hooks_it = on_create_entity_hooks.find(new_type); on_create_entity_hooks_it != on_create_entity_hooks.end()) {
            on_create_entity_hooks_it->second.insert(on_create_entity_hooks_it->second.end(), std::move(callback));
        } else {
            on_create_entity_hooks.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(new_type),
                std::forward_as_tuple(std::move(callback))
            );
        }
    }

    void add_on_remove_component_hooks(const Type new_type, std::initializer_list<std::function<void(OnRemoveComponentHook, World&, const Entity)>>&& callback) noexcept {
        if (auto on_remove_component_hooks_it = on_remove_component_hooks.find(new_type); on_remove_component_hooks_it != on_remove_component_hooks.end()) {
            on_remove_component_hooks_it->second.insert(on_remove_component_hooks_it->second.end(), std::move(callback));
        } else {
            on_remove_component_hooks.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(new_type),
                std::forward_as_tuple(std::move(callback))
            );
        }
    }

    void add_on_delete_entity_hooks(const Type new_type, std::initializer_list<std::function<void(OnDeleteEntityHook, World&, const Entity)>>&& callback) noexcept {
        if (auto on_delete_entity_hooks_it = on_delete_entity_hooks.find(new_type); on_delete_entity_hooks_it != on_delete_entity_hooks.end()) {
            on_delete_entity_hooks_it->second.insert(on_delete_entity_hooks_it->second.end(), std::move(callback));
        } else {
            on_delete_entity_hooks.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(new_type),
                std::forward_as_tuple(std::move(callback))
            );
        }
    }

    void start(World& world) const noexcept {
        for (const auto& func: startSystems) {
            func(start_system, world);
        }
    }

    void run(World& world) noexcept {
        for (const auto& mainFunc: mainSystems) {
            if (mainFunc.first == nullptr || mainFunc.first(world)) {
                for (const auto& mainRow: mainFunc.second) {
                    mainRow(main_system, world);
                }
            }
        }

        for (const auto& funcs: threadedSystems) {
            if (funcs.first == nullptr || funcs.first(world)) {
                if (!isUseMultithreading) {
                    for (auto& func: funcs.second) {
                        func(threaded_system, world);
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
                    lateRow(late_system, world);
                }
            }
        }
    }

    void runThreadedFixedSetRec(World& world, const ThreadedFixedSet& set) noexcept {
        if (set.condition == nullptr || set.condition(world)) {
            if (!set.tasks.empty()) {
                if (!isUseMultithreading) {
                    for (auto& func: set.tasks) {
                        func(threaded_fixed_system, world);
                    }
                } else {
                    threadpool.addFixedTasks(set.tasks);
                }
            }
            for (const auto& subSet: set.subSets) {
                runThreadedFixedSetRec(world, subSet);
            }
        }
    }

    void runFixed(World& world) noexcept {
        for (const auto& [condition, systems]: mainFixedSystems) {
            if (condition == nullptr || condition(world)) {
                for (const auto& system: systems) {
                    system(main_fixed_system, world);
                }
            }
        }

        for (const auto& subSet: threadedFixedSystems) {
            runThreadedFixedSetRec(world, subSet);
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
                    mainRow(main_unscaled_fixed_system, world);
                }
            }
        }

        for (const auto& funcs: threadedUnscaledFixedSystems) {
            if (funcs.first == nullptr || funcs.first(world)) {
                if (!isUseMultithreading) {
                    for (auto& func: funcs.second) {
                        func(threaded_unscaled_fixed_system, world);
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
                    lateRow(late_unscaled_fixed_system, world);
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

public:
    std::unordered_map<Type, std::vector<std::function<void(OnAddComponentHook, World&, const Entity)>>> on_add_component_hooks;
    std::unordered_map<Type, std::vector<std::function<void(OnCreateEntityHook, World&, const Entity)>>> on_create_entity_hooks;
    std::unordered_map<Type, std::vector<std::function<void(OnRemoveComponentHook, World&, const Entity)>>> on_remove_component_hooks;
    std::unordered_map<Type, std::vector<std::function<void(OnDeleteEntityHook, World&, const Entity)>>> on_delete_entity_hooks;

private:
    ThreadPool threadpool;
    bool isUseMultithreading {true};
};

constexpr void LateUpgrade::upgrade_hook_add_component(World& world, Sys& sys, const Entity entity, const Type type) noexcept {
    if (auto hooks_it = sys.on_add_component_hooks.find(type); hooks_it != sys.on_add_component_hooks.end()) {
        for (const auto& callback: hooks_it->second) {
            callback({}, world, entity);
        }
    }
}

constexpr void LateUpgrade::upgrade_hook_create_entity_with_component(World& world, Sys& sys, const Entity entity, const Type type) noexcept {
    if (auto hooks_it = sys.on_create_entity_hooks.find(type); hooks_it != sys.on_create_entity_hooks.end()) {
        for (const auto& callback: hooks_it->second) {
            callback({}, world, entity);
        }
    }
}

constexpr void LateUpgrade::upgrade_hook_remove_component(World& world, Sys& sys, const Entity entity, const Type type) noexcept {
    if (auto hooks_it = sys.on_remove_component_hooks.find(type); hooks_it != sys.on_remove_component_hooks.end()) {
        for (const auto& callback: hooks_it->second) {
            callback({}, world, entity);
        }
    }
}

constexpr void LateUpgrade::upgrade_hook_delete_entity_with_component(World& world, Sys& sys, const Entity entity, const Type type) noexcept {
    if (auto hooks_it = sys.on_delete_entity_hooks.find(type); hooks_it != sys.on_delete_entity_hooks.end()) {
        for (const auto& callback: hooks_it->second) {
            callback({}, world, entity);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////

class [[nodiscard]] Time final: public IResource {
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

class [[nodiscard]] World final {
friend class ZerEngine;
private:
    World() noexcept:
        sys(*this) {
    }

public:
    [[nodiscard("La valeur de retour d'une commande Exist doit toujours etre evalue")]] auto is_entity_exists(const Entity entity) const noexcept -> bool {
        return (reg.is_entity_exist(entity) || lateUpgrade.add_entities.contains(entity)) && !lateUpgrade.delEnts.contains(entity);
    }

    template <typename T, typename... Ts> requires ((IsComponentConcept<T> && (IsComponentConcept<Ts> && ...)) && (!std::is_const_v<T> || (!std::is_const_v<Ts> || ...)))
    [[nodiscard("La valeur de retour d'une commande Has doit toujours etre evalue")]] auto has_components_this_frame(const Entity entity) const noexcept -> bool {
        if (!is_entity_exists(entity)) {
            return false;
        }
        if (reg.has_components(entity, {typeid(T).hash_code()})) {
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
        return has_components_this_frame<Ts...>(entity);
    }

private:
    [[nodiscard]] auto internal_get_components_this_frame(const Entity entity, const Type type) noexcept -> std::optional<std::reference_wrapper<std::unique_ptr<IComponent>>> {
        if (auto addCompsIt = lateUpgrade.addComps.find(entity); addCompsIt != lateUpgrade.addComps.end()) {
            if (addCompsIt->second.contains(type)) {
                return lateUpgrade.registry_messages[addCompsIt->second.at(type)].component.second;
            }
        }
        if (auto opt = reg.get(entity, type)) {
            return opt.value();
        }
        return std::nullopt;
    }

public:
    template <typename T, typename... Ts> requires (IsComponentConcept<T> && IsNotEmptyConcept<T> && IsNotSameConcept<T, Ts...>)
    [[nodiscard("La valeur de retour d'une commande Get doit toujours etre recupere")]] auto get_components_this_frame(const Entity entity) noexcept -> std::optional<std::tuple<T&, Ts&...>> {
        if (auto opt_component = internal_get_components_this_frame(entity, typeid(T).hash_code())) {
            auto& component = *static_cast<T*>(opt_component.value().get().get());
            if constexpr (sizeof...(Ts) > 0) {
                if (auto othOpt = get_components_this_frame<Ts...>(entity)) {
                    return std::tuple_cat(std::forward_as_tuple(component), othOpt.value());
                }
            } else {
                return std::forward_as_tuple(component);
            }
        }
        return std::nullopt;
    }

    template <typename... Ts> requires (sizeof...(Ts) > 0 && ((IsComponentConcept<Ts> && IsNotEmptyConcept<Ts>) && ...) && IsNotSameConcept<Ts...>)
    [[nodiscard("La valeur de retour d'une commande Get doit toujours etre recupere")]] auto get_components(const Entity entity) noexcept -> std::optional<std::tuple<Ts&...>> {
        return get_components_this_frame<Ts...>(entity);
    }

    constexpr auto append_children(const Entity parent_entity, const std::vector<Entity>& children_entity) noexcept -> Entity {
        lateUpgrade.append_children(reg, parent_entity, children_entity);
        return parent_entity;
    }

    template <typename... Ts> requires ((sizeof...(Ts) > 0) && (IsResourceConcept<Ts> && ...))
    [[nodiscard("La valeur de retour d'une commande Resource doit toujours etre recupere")]] auto resource() noexcept -> std::tuple<Ts&...> {
        return std::forward_as_tuple(*static_cast<Ts*>(res.get(typeid(Ts).hash_code()).get())...);
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

public:
    template <typename... Components> requires ((IsComponentConcept<Components> && ...) && IsNotSameConcept<Components...>)
    auto create_entity(Components&&... components) noexcept -> Entity {
        auto entity_token = reg.get_entity_token();
        lateUpgrade.create_entity(entity_token);
        (lateUpgrade.add_components(
            reg,
            entity_token,
            {
                typeid(Components).hash_code(),
                std::is_empty_v<Components> ? nullptr : std::make_unique<Components>(std::move(components))
            },
            typeid(Components).name()
        ), ...);
        return entity_token;
    }

    template <typename... Components> requires ((IsComponentConcept<Components> && ...) && IsNotSameConcept<Components...>)
    void add_components(const Entity entity, Components&&... components) noexcept {
        if (is_entity_exists(entity)) {
            (lateUpgrade.add_components(
                reg,
                entity,
                {
                    typeid(Components).hash_code(),
                    std::is_empty_v<Components> ? nullptr : std::make_unique<Components>(std::move(components))
                },
                typeid(Components).name()
            ), ...);
        } else if (!lateUpgrade.delEnts.contains(entity)) {
            (std::println("World::add_components(): Impossible d'ajouter sur une entitée qui n'existe pas [Entity: {}], [type: {}]", entity, typeid(Components).name()), ...);
        }
    }

    template <typename... Components> requires ((IsComponentConcept<Components> && ...) && IsNotSameConcept<Components...>)
    void remove_components(const Entity entity) noexcept {
        if (is_entity_exists(entity)) {
            lateUpgrade.remove_components(reg, entity, {{typeid(Components).name(), typeid(Components).hash_code()}...});
        } else if (!lateUpgrade.delEnts.contains(entity)) {
            (std::println("World::remove_components(): Impossible de supprimer un composant qui n'existe pas - [Entity: {}], [type: {}]", entity, typeid(Components).name()), ...);
        }
    }

    void delete_entity(const Entity entity) noexcept {
        if (is_entity_exists(entity)) {
            lateUpgrade.delete_entity(reg, entity);
        } else if (!lateUpgrade.delEnts.contains(entity)) {
            std::println("World::delete_entity(): Impossible de supprimer une entitée qui n'existe pas - [Entity: {}]", entity);
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

    void add_dont_destroy_on_load(const Entity ent) noexcept {
        lateUpgrade.add_dont_destroy_on_load(ent);
    }

    void load_scene(void(*const new_scene)(SceneSystem, World&)) noexcept {
        lateUpgrade.load_scene(new_scene);
    }

    void stop_run(bool val = true) noexcept {
        isRunning = !val;
    }

    void upgrade() noexcept {
        lateUpgrade.upgrade(*this, reg, sys);
    }

private:
    TypeMap res;
    Registry reg;
    LateUpgrade lateUpgrade;
    Sys sys;
    bool isRunning;
};

///////////////////////////////////////////////////////////////////////////////////

class [[nodiscard]] ZerEngine final {
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

    template <typename Component>
    [[nodiscard]] auto add_hooks(OnAddComponentHook, std::initializer_list<std::function<void(OnAddComponentHook, World&, const Entity)>>&& callback) noexcept -> ZerEngine& {
        world.sys.add_on_add_component_hooks(typeid(Component).hash_code(), std::move(callback));
        return *this;
    }

    template <typename Component>
    [[nodiscard]] auto add_hooks(OnCreateEntityHook, std::initializer_list<std::function<void(OnCreateEntityHook, World&, const Entity)>>&& callback) noexcept -> ZerEngine& {
        world.sys.add_on_create_entity_hooks(typeid(Component).hash_code(), std::move(callback));
        return *this;
    }

    template <typename Component>
    [[nodiscard]] auto add_hooks(OnRemoveComponentHook, std::initializer_list<std::function<void(OnRemoveComponentHook, World&, const Entity)>>&& callback) noexcept -> ZerEngine& {
        world.sys.add_on_remove_component_hooks(typeid(Component).hash_code(), std::move(callback));
        return *this;
    }

    template <typename Component>
    [[nodiscard]] auto add_hooks(OnDeleteEntityHook, std::initializer_list<std::function<void(OnDeleteEntityHook, World&, const Entity)>>&& callback) noexcept -> ZerEngine& {
        world.sys.add_on_delete_entity_hooks(typeid(Component).hash_code(), std::move(callback));
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
                        world.sys.run_callbacks(world);
                        world.upgrade();
                        world.sys.runFixed(world);
                    }
                }
                for (unsigned int i = 0; i < time.get_nb_fixed_steps(); i++) {
                    world.sys.run_callbacks(world);
                    world.upgrade();
                    world.sys.runUnscaledFixed(world);
                }
            }

            world.sys.run_callbacks(world);
            world.upgrade();
            world.sys.runLate(world);

            world.sys.run_callbacks(world);
            world.upgrade();
        }
    }

private:
    World world;
};

///////////////////////////////////////////////////////////////////////////////////
