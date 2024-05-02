module;

#include <unordered_map>
#include <any>
#include <vector>
#include <unordered_set>

export module ZerengineCore:Registry;

import :Utils;
import :Archetype;
import :View;

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

    [[nodiscard]] auto&& get(this auto&& self, const Ent ent, const Type type) noexcept(false) {
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

    void filterArchsByType(const Type type, std::unordered_set<Archetype*>& archs) noexcept {
        std::unordered_set<Archetype*> newArchs;
        if (auto archsByTypeIt = archsByType.find(type); archsByTypeIt != archsByType.end()) {
            for (auto* arch: archsByTypeIt->second) {
                if (archs.contains(arch)) {
                    newArchs.emplace(arch);
                }
            }
        }
        archs = newArchs;
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