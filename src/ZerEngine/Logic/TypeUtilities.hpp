/**
 * @file TypeUtilities.hpp
 * @author ZerehtjiN
 * @brief Definition of several type to help
 * developer.
 * @version 0.1
 * @date 2022-02-22
 * 
 * @copyright Copyright (c) 2022 - ZerethjiN
 * 
 */
#ifndef ZERENGINE_TYPE_UTILITIES_HPP
#define ZERENGINE_TYPE_UTILITIES_HPP

#include <cstdint>
#include "TypeMap.hpp"

namespace zre {
    namespace priv {
        template <typename... Excludes>
        struct Without {
            constexpr static auto size = sizeof...(Excludes);
        };

        template <typename... Filters>
        struct With {
            constexpr static auto size = sizeof...(Filters);
        };

        template <typename... Comps>
        struct comp_t {
            constexpr static auto size = sizeof...(Comps);
        };
    }

    template <typename... Excludes>
    constexpr priv::Without<Excludes...> without;

    template <typename... Filters>
    constexpr priv::With<Filters...> with;

    using Type = size_t;

    #ifdef ZER_ENT_64BITS
        using Ent = uint64_t;
    #else
        using Ent = uint32_t;
    #endif

    namespace priv {
        using Res = TypeMap;
    }
}

#endif // ZERENGINE_TYPE_UTILITIES_HPP