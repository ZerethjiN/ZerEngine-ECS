module;

#include <unordered_map>
#include <any>
#include <unordered_set>
#include <mutex>

export module ZerengineCore:LateUpgrade;

import :Utils;
import :Registry;

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
        if (needClean) {
            needClean = false;
            delEnts.clear();
            addEnts.clear();
            delComps.clear();
            addComps.clear();
            reg.clean();
            newSceneFunc(world);
        }

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
    }

private:
    std::mutex mtx;
    std::unordered_map<Ent, std::unordered_map<Type, std::any>> addEnts;
    std::unordered_map<Ent, std::unordered_map<Type, std::any>> addComps;
    std::unordered_set<Ent> delEnts;
    std::unordered_map<Ent, std::unordered_set<Type>> delComps;
    bool needClean = false;
    void(*newSceneFunc)(World&);
};