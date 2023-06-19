# ZerEngine-ECS
![Logo](https://github.com/ZerethjiN/ZerEngine-ECS/blob/main/LogoZerEngineBlanc.png)

A simple ECS logic core.

* Cache Friendly Component managed by Archetypal.
* Dynamic resource added by user and easy to access.
* Simple implementation of components without inheritance.
* Application easy to build and run.
* Multi Threading on systems.

# Code Example
```c++
#include <ZerEngine/ZerEngine.hpp>

// New FakeTime resource.
class FakeTime {
public:
    float fakeDelta() {
        return 1;
    }
};

// Components declaration.
struct Pos {
    float x;
    float y;
};

struct Vel {
    float x;
    float y;
};

// Initialization system executed only once at startup.
void initPos(zre::World& world) {
    for (float i = 0; i < 5; i++) {
        world.newEnt(
            Pos { i + 10.f, i + 20.f },
            Vel { 10.f, 10.f }
        );
    }
}

// Systems executed on each frame.
void movePosSys(zre::World& world) {
    auto positions = world.query<Pos, Vel>();
    const auto& time = world.getRes<FakeTime>();

    positions.each([&](auto& pos, const auto& vel) {
        pos.x += vel.x * time.fakeDelta();
        pos.y += vel.y * time.fakeDelta();
    });
}

void secondThreadedSys(zre::Wordl& world) {
    /* My second thread */
}

void stopRunSys(zre::World& world) {
    world.stopRun();
}

// Render System
void renderCopySys(zre::World& world, zre::priv::LiteRegistry& reg) noexcept {
    reg.clear();

    reg.copyFromQuery(world.query</*Sprite, Transform, ...*/>());
}

void renderSys(zre::World& world, zre::priv::LiteRegistry& renderReg) noexcept {
    auto sprts = renderReg.query</*Sprite, Transform, ...*/>();
    
    sprts.each([&](/*auto& Sprite, auto& Transform, ...*/) {
        /* My Render */
    });
}

int main() {
    // Our application.
    zre::ZerEngine()
        .addRes<FakeTime>()
        .addStartSys(initPos)
        .addSys(movePosSys, secondThreadedSys)
        .addSys(stopRunSys)
        .addLateSys(/*...*/)
        .addRenderCopy(renderCopySys)
        .addRender(renderSys)
        .run();

    return 0;
}
```

# Integration
Pass -I argument to the compiler to add the src directory to the include paths.
```c++
#include <ZerEngine/ZerEngine.hpp>
```
