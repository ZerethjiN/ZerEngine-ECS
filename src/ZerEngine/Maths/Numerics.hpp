#ifndef ZERENGINE_MATHS_NUMERICS
#define ZERENGINE_MATHS_NUMERICS

#include <cstdint>

namespace zre {
    using u8 = uint_fast8_t;
    using u16 = uint_fast16_t;
    using u32 = uint_fast32_t;
    using u64 = uint_fast64_t;

    using i8 = int_fast8_t;
    using i16 = int_fast16_t;
    using i32 = int_fast32_t;
    using i64 = int_fast64_t;

    using f32 = float;
    using f64 = double;

    template <typename T1, typename T2>
    constexpr T1 cast(T2 value) noexcept {
        return static_cast<T1>(value);
    }
}

#endif