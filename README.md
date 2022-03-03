# ZerEngine-ECS
![Logo](https://github.com/ZerethjiN/ZerEngine-ECS/blob/main/LogoZerEngineBlanc.png)

A simple ECS logic core.

* Cache Friendly Component managed by a sparse set.
* Dynamic resource added by user and easy to access.
* Possibility to used 64bits of entity instead of 32bits with ``#define ZER_ENT_64BITS``.
* Simple implementation of components without inheritance.
* Application easy to build and run.

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

void stopRunSys(zre::World& world) {
    world.stopRun();
}

int main() {
    // Our application.
    zre::ZerEngine()
        .addRes<FakeTime>()
        .addStartSys(initPos)
        .addSys(movePosSys)
        .addSys(stopRunSys)
        .run();

    return 0;
}
```

# Integration
Pass -I argument to the compiler to add the src directory to the include paths.
```c++
#include <ZerEngine/ZerEngine.hpp>
```

# Future Features
* Multi Threading on systems.
* Change sparseset by archetypal/sparseset hybrid logic.

# Links
* [Twitch](https://www.twitch.tv/zerethjin)
* [Youtube](https://www.youtube.com/channel/UCD2oEhxIyDYdS8-RhF1GoRg)
* [TikTok](https://www.tiktok.com/@zerethjin)
