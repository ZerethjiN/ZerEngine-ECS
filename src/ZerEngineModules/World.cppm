module;

#include <vector>
#include <string>
#include <optional>
#include <any>
#include <unordered_set>

export module ZerengineCore:World;

import :Utils;
import :TypeMap;
import :Registry;
import :LateUpgrade;
import :Sys;
import :View;

export class World final {
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
    [[nodiscard]] T& internalGet(const Ent ent) noexcept(false) {
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
        throw std::exception();
    }

    template <typename T>
    [[nodiscard]] const T& internalGet(const Ent ent) const noexcept(false) {
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
        throw std::exception();
    }

public:
    template <typename... Ts> requires (sizeof...(Ts) >= 1)
    [[nodiscard]] std::optional<std::tuple<Ts&...>> get(const Ent ent) noexcept {
        try {
            return std::forward_as_tuple(internalGet<Ts>(ent)...);
        } catch(std::exception) {
            return std::nullopt;
        }
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
        reg.appendChildren(parentEnt, childrenEnt);
    }

    template <typename... Ts> requires (sizeof...(Ts) > 0)
    [[nodiscard]] decltype(auto) getRes(this auto&& self) noexcept {
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
    std::tuple<Comp&, Comps&...> add(const Ent ent, const Comp& comp, const Comps&... comps) noexcept(false) {
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
            throw "World Add() Error";
        }

        return std::forward_as_tuple(internalGet<Comp>(ent), internalGet<Comps>(ent)...);
    }

    template <typename T, typename... Ts>
    void del(const Ent ent) noexcept(false) {
        if (reg.has(ent, {typeid(T).hash_code()})) {
            lateUpgrade.del(ent, typeid(T).hash_code());
        } else {
            printf("World::del(): Impossible de supprimer un composant qui n'existe pas - %s\n", typeid(T).name());
            throw "World Del() Error";
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
        }
    }

    void setInactive(const Ent ent) noexcept {
        if (!has<IsInactive>(ent)) {
            add(ent, IsInactive());
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