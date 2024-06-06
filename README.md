# ZerEngine-ECS
![Logo](https://github.com/ZerethjiN/ZerEngine-ECS/blob/main/LogoZerEngineBlanc.png)

A simple ECS logic core.

* /!\ C++26 Minimum /!\
* Cache Friendly Component managed by Archetypal.
* Dynamic resource added by user and easy to access.
* Simple implementation of components without inheritance.
* Application easy to build and run.
* Multi Threading on systems.

# Code Example
```c++
#include <Zerengine.hpp>

// Ressouces declaration.
struct AppState {
    enum AppStateType: size_t {
        HOME_SCREEN,
        IN_GAME
    };

    AppState(AppStateType newCurAppState):
        curAppState(newCurAppState) {
    }

    AppStateType curAppState;
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
void initPos(StartSystem, World& world) {
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
void movePosSys(ThreadedFixedSystem, World& world) {
    auto positions = world.view<Position, const Velocity>();
    auto [time] = world.resource<const Time>();

    for (auto [_, position, velocity]: positions) {
        position.x += velocity.x * time.fixedDelta();
        position.y += velocity.y * time.fixedDelta();
    }
}

void playerActionSys(ThreadedSystem, World& world) {
    auto players = world.view(with<Player>, without<PlayerDash>);

    for (auto [playerEnt]: players) {
        if (/*Dash Button Pressed*/) {
            world.add(playerEnt, PlayerDash{0.5f, 0.0f, 8.0f});
        }
    }
}

void playerDashSys(ThreadedSystem, World& world) {
    auto players = world.view<PlayerDash, Velocity>();
    auto [time] = world.resource<const Time>();

    for (auto [playerEnt, playerDash, velocity]: players) {
        if (playerDash.canStopDash(time.deltaTime())) {
            world.del<PlayerDash>(playerEnt);
            velocity.x = 0;
        } else {
            velocity.x += playerDash.dashSpeed;
        }
    }
}

void stopRunSys(MainSystem, World& world) {
    world.stopRun();
}

int main() {
    // Our application.
    ZerEngine()
        .useMultithreading(true) // <== optional
        .setFixedTimeStep(0.02f) // <== Set fixed time step for fixed systems
        .addRes<AppState>(AppState::IN_GAME)
        .addStartSys(initPos)
        .addMainSys(stopRunSys)
        .addThreadedSys(playerActionSys, playerDashSys) // Systems work at the same time
        .addThreadedFixedSys(movePosSys) // <== Systems work at fixed time
        .addThreadedCondSys(
            [](World& world) -> bool {
                auto [appState] = world.getRes<const AppState>();
                return appState == AppState::IN_GAME;
            },
            /** This Systems only runs if condition is true **/
        )
        .addLateSys(/*...*/)
        .run();

    return 0;
}
```

# Integration
Pass -I argument to the compiler to add the src directory to the include paths.
```c++
#include <ZerEngine.hpp>
```