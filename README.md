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
import ZerengineCore;

// New FakeTime resource.
struct FakeTime {
    float fakeDelta() const {
        return 1;
    }
};

// Components declaration.
struct Position {
    float x;
    float y;
};

struct Velocity {
    float x;
    float y;
};

struct Player {};

struct PlayerDash {
    float cooldown;
    float curTime;
    float dashSpeed;

    bool canStopDash(float delta) {
        curTime += delta;
        return curTime >= cooldown;
    }
};

// Initialization system executed only once at startup.
void initPos(World& world) {
    world.newEnt(
        Player{},
        Position{0.0f, 0.0f},
        Velocity{0.0f, 0.0f}
    );

    for (float i = 0; i < 5; i++) {
        world.newEnt(
            Position{i + 10.f, i + 20.f},
            Velocity{10.f, 10.f}
        );
    }
}

// Systems executed on each frame.
void movePosSys(World& world) {
    auto positions = world.view<Position, const Velocity>();
    auto [time] = world.getRes<const FakeTime>();

    for (auto [_, position, velocity]: positions) {
        position.x += velocity.x * time.fakeDelta();
        position.y += velocity.y * time.fakeDelta();
    }
}

void playerActionSys(World& world) {
    auto players = world.view(with<Player>, without<PlayerDash>);

    for (auto [playerEnt]: players) {
        if (/*Dash Button Pressed*/) {
            world.add(playerEnt, PlayerDash{0.5f, 0.0f, 8.0f});
        }
    }
}

void playerDashSys(World& world) {
    auto players = world.view<PlayerDash, Velocity>();
    auto [time] = world.getRes<const FakeTime>();

    for (auto [playerEnt, playerDash, velocity]: players) {
        if (playerDash.canStopDash(time.fakeDelta())) {
            world.del<PlayerDash>(playerEnt);
            velocity.x = 0;
        } else {
            velocity.x += playerDash.dashSpeed;
        }
    }
}

void stopRunSys(World& world) {
    world.stopRun();
}

int main() {
    // Our application.
    ZerEngine()
        .addRes<FakeTime>()
        .addStartSys(initPos)
        .addSys(playerActionSys, playerDashSys) // Systems work at the same time
        .addSys(movePosSys, stopRunSys)
        .addLateSys(/*...*/)
        .run();

    return 0;
}
```

# Integration (One File)
Pass -I argument to the compiler to add the src directory to the include paths.
```c++
#include <ZerEngine/ZerEngine.hpp>
```

# Integration (Modules)
Precompile the modules in the ZerEngineModules folder and import them with:
```c++
import ZerengineCore;
```