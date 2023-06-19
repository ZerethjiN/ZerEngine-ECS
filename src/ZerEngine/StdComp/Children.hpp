#ifndef ZERENGINE_STDCOMP_CHILDREN_HPP
#define ZERENGINE_STDCOMP_CHILDREN_HPP

#include <vector>
#include <cassert>
#include "../Logic/ZerEngine.hpp"
#include "Parent.hpp"

struct Children {
public:
    Children(std::initializer_list<zre::Ent> list = {}):
        children(std::make_shared<std::vector<zre::Ent>>(list)) {
    }

    template <typename... Args>
    constexpr void addChildren(const zre::Ent& newChild, const Args&... newChildren) noexcept {
        children->push_back(newChild);
        if constexpr(sizeof...(Args) > 0)
            addChildren(std::forward<Args...>(newChildren...));
    }

    constexpr zre::Ent getChild(size_t idx) const noexcept {
        assert(((void)"Impossible to get child", idx < children->size()));
        return children->at(idx);
    }

    constexpr zre::Ent operator [](size_t idx) const noexcept {
        assert(((void)"Impossible to get child", idx < children->size()));
        return children->at(idx);
    }

    constexpr size_t childCount() const noexcept {
        return children->size();
    }

    constexpr void detachChildren() noexcept {
        children->clear();
    }

    constexpr std::vector<zre::Ent>::const_iterator begin() const noexcept {
        return children->begin();
    }

    constexpr std::vector<zre::Ent>::const_iterator end() const noexcept {
        return children->end();
    }

public:
    std::shared_ptr<std::vector<zre::Ent>> children;
};

namespace zre::priv {
    template <typename... Ents>
    void addParent(World& world, Ent entParent, Ent entChild, Ents... children) {
        world.add(entChild, Parent(entParent));
        if constexpr (sizeof...(Ents) > 0) {
            addParent(world, entParent, children...);
        }
    }
}

template <typename... Ents>
void appendChildren(zre::World& world, zre::Ent entParent, const Ents&... children) {
    world.add(entParent, Children({children...}));
    if constexpr (sizeof...(Ents) > 0) {
        addParent(world, entParent, children...);
    }
}

#endif /* ZERENGINE_STDCOMP_CHILDREN_HPP */