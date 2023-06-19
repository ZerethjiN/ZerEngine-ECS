#ifndef ZERENGINE_STDCOMP_PARENT_HPP
#define ZERENGINE_STDCOMP_PARENT_HPP

#include "../Logic/ZerEngine.hpp"

struct Parent {
public:
    Parent(zre::Ent newParent):
        parent(newParent) {
    }

public:
    zre::Ent parent;
};

#endif /* ZERENGINE_STDCOMP_PARENT_HPP */