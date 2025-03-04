# ZerEngine-ECS
![Logo](https://github.com/ZerethjiN/ZerEngine-ECS/blob/main/LogoZerEngineBlanc.png)

A simple ECS logic core.

* /!\ C++23 Minimum /!\
* Cache Friendly Component managed by Archetypal.
* Dynamic resource added by user and easy to access.
* Simple implementation of components without inheritance.
* Application easy to build and run.
* Multi Threading on systems.

# Code Example
```c++
#include <Zerengine.hpp>

// Ressouces declaration.
struct [[nodiscard]] AppState final: public IResource {
public:
    enum class AppStateType: size_t {
        HOME_SCREEN,
        IN_GAME
    };

public:
    constexpr AppState(AppStateType new_cur_app_state) noexcept:
        cur_app_state(new_cur_app_state) {
    }

public:
    AppStateType cur_app_state;
};

// Components declaration.
struct [[nodiscard]] Position final: public IComponent {
public:
    constexpr Position(float new_x, float new_y) noexcept:
        x(new_x),
        y(new_y) {
    }

private:
    float x;
    float y;
};

struct [[nodiscard]] Velocity final: public IComponent {
public:
    constexpr Velocity(float new_x, float new_y) noexcept:
        x(new_x),
        y(new_y) {
    }

private:
    float x;
    float y;
};

struct [[nodiscard]] Player final: public IComponent {};

struct [[nodiscard]] PlayerDash final: public IComponent {
public:
    constexpr PlayerDash(float new_cooldown, float new_dash_speed) noexcept:
        cooldown(new_cooldown),
        cur_time(0),
        dash_speed(new_dash_speed) {
    }

public:
    [[nodiscard]] constexpr auto can_stop_dash(float delta) noexcept -> bool {
        cur_time += delta;
        return cur_time >= cooldown;
    }

private:
    float cooldown;
    float cur_time;
    float dash_speed;
};

// Initialization system executed only once at startup.
constexpr void init_pos(StartSystem, World& world) noexcept {
    world.create_entity(
        Player(),
        Position(
            /*x:*/ 0.0f,
            /*y:*/ 0.0f
        ),
        Velocity(
            /*x:*/ 0.0f,
            /*y:*/ 0.0f
        )
    );

    for (float i = 0; i < 5; i += 1.0f) {
        world.create_entity(
            Position(
                /*x:*/ i + 10.0f,
                /*y:*/ i + 20.0f
            ),
            Velocity(
                /*x:*/ 10.0f,
                /*y:*/ 10.0f
            )
        );
    }
}

// Systems executed on each frame.
constexpr void move_pos_sys(ThreadedFixedSystem, World& world) noexcept {
    auto positions = world.query<Position, const Velocity>();

    auto [time] = world.resource<const Time>();

    for (auto [_, position, velocity]: positions) {
        position.x += velocity.x * time.fixedDelta();
        position.y += velocity.y * time.fixedDelta();
    }
}

constexpr void player_action_sys(ThreadedSystem, World& world) noexcept {
    auto players = world.query(with<Player>, without<PlayerDash>);

    for (auto [player_ent]: players) {
        if (/*Dash Button Pressed*/) {
            world.add_components(player_ent,
                PlayerDash(
                    /*cooldown:*/ 0.5f,
                    /*dash_speed:*/ 8.0f
                )
            );
        }
    }
}

constexpr void player_dash_sys(ThreadedSystem, World& world) noexcept {
    auto players = world.query<PlayerDash, Velocity>();

    auto [time] = world.resource<const Time>();

    for (auto [player_ent, player_dash, velocity]: players) {
        if (playerDash.can_stop_dash(time.deltaTime())) {
            world.remove_components<PlayerDash>(playerEnt);
            velocity.x = 0;
        } else {
            velocity.x += playerDash.dash_speed;
        }
    }
}

constexpr void stop_run_sys(MainSystem, World& world) noexcept {
    world.stop_run();
}

auto main() -> int {
    // Our application.
    ZerEngine()
        .use_multithreading(true) // <== optional
        .set_fixed_time_step(0.02f) // <== Set fixed time step for fixed systems
        .add_resource<AppState>(AppState::IN_GAME)
        .add_systems(startSystem, init_pos)
        .add_systems(mainSystem, stop_run_sys)
        .add_systems(threadedFixedSystems,
            {
                player_action_sys, player_dash_sys
            }
        ) // Systems work at the same time
        .add_systems(threadedFixedSystem,
            {
                move_pos_sys
            }
        ) // <== Systems work at fixed time
        .add_systems(threadedFixedSystem,
            [](World& world) -> bool {
                auto [app_state] = world.resource<const AppState>();
                return app_state == AppState::IN_GAME;
            },
            {
                /** This Systems only runs if condition is true **/
            }
        )
        .add_systems(lateSystem,
            {
                /*...*/
            }
        )
        .run();

    return 0;
}
```

# Integration
Pass -I argument to the compiler to add the src directory to the include paths.
```c++
#include <ZerEngine.hpp>
```