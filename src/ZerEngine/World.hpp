#pragma once

#include "TypeMap.hpp"
#include "Sys.hpp"
#include "Registry.hpp"
#include "LateUpgrade.hpp"

class World final {
public:
    inline World() noexcept:
        sys(*this) {
    }

    template <typename T>
    [[nodiscard]] constexpr bool has(const Ent ent) const noexcept {
        return reg.has<T>(ent);
    }

    template <typename T>
    [[nodiscard]] constexpr T& get(const Ent ent) noexcept {
        return reg.get<T>(ent);
    }

    template <typename T>
    [[nodiscard]] constexpr const T& get(const Ent ent) const noexcept {
        return reg.get<T>(ent);
    }

    template <typename T>
    [[nodiscard]] constexpr T& getRes() noexcept {
        return res.get<T>();
    }

    template <typename T>
    [[nodiscard]] constexpr const T& getRes() const noexcept {
        return res.get<T>();
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    [[nodiscard]] constexpr const View<Comps...> view(const With<Filters...> filters = {}, const Without<Excludes...> excludes = {}) {
        return reg.view<Comps...>(sys, filters, excludes);
    }

    template <typename... Comps, typename... Filters, typename... Excludes>
    [[nodiscard]] constexpr const View<Comps...> view(const Without<Excludes...> excludes, const With<Filters...> filters = {}) {
        return reg.view<Comps...>(sys, filters, excludes);
    }

    template <typename... Comps>
    constexpr Ent newEnt(const Comps&... comps) noexcept {
        return lateUpgrade.newEnt(reg, comps...);
    }

    template <typename Arg>
    constexpr void add(const Ent ent, const Arg& arg) noexcept {
        lateUpgrade.add(reg, ent, arg);
    }

    template <typename T>
    constexpr void del(const Ent ent) noexcept {
        lateUpgrade.del<T>(ent);
    }

    template <typename Old, typename New>
    constexpr void replace(const Ent ent, const New& newComp) noexcept {
        lateUpgrade.add(reg, ent, newComp);
        lateUpgrade.del<Old>(ent);
    }

    constexpr void destroy(const Ent ent) noexcept {
        lateUpgrade.destroy(ent);
    }

    constexpr void stopRun(bool val = true) noexcept {
        isRunning = !val;
    }

    constexpr void upgrade() noexcept {
        lateUpgrade.upgrade(reg);
    }

public:
    TypeMap res;
    Sys sys;
    Registry reg;
    LateUpgrade lateUpgrade;
    bool isRunning;
};