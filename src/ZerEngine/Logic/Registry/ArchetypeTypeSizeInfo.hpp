#pragma once

#include <cstdlib>
#include <tuple>

using namespace std;

namespace zre {
    using Type = size_t;
    using Ent = size_t;

    namespace priv {
        struct TypeInfo {
            size_t size;
            size_t offset;

            constexpr TypeInfo(size_t newSize = 0, size_t newOffset = 0) noexcept:
                size(newSize),
                offset(newOffset) {
            }
        };

        struct AbstractWith {};

        struct AbstractWithout {};
    }

    template <typename... Filters>
    struct With: public priv::AbstractWith {
        typedef std::tuple<Filters...> tup;
    };

    template <typename... Filters>
    constexpr With<Filters...> with;

    template <typename... Excludes>
    struct Without: public priv::AbstractWithout {
        typedef std::tuple<Excludes...> tup;
    };

    template <typename... Excludes>
    constexpr Without<Excludes...> without;
}