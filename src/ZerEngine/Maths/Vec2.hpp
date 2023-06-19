#ifndef ZERENGINE_MATHS_VEC2
#define ZERENGINE_MATHS_VEC2

#include <cstdint>

#ifdef __SSE2__
    #include <immintrin.h>
#elif __ARM_NEON
    #include <arm_neon.h>
#endif

namespace zre {
    template <typename T>
    struct Vec2Datas {
        typedef T data[2];
    };

    #ifdef __ARM_NEON
        template<>
        struct Vec2Datas<float> {
            typedef float32x2_t data;
        };

        template<>
        struct Vec2Datas<int32_t> {
            typedef int32x2_t data;
        };
    #endif

    template <typename T>
    struct Vec2T {
    public:
        #ifdef __clang__
            #pragma clang diagnostic push
            #pragma clang diagnostic ignored "-Wgnu-anonymous-struct"
            #pragma clang diagnostic push
            #pragma clang diagnostic ignored "-Wnested-anon-types"
        #elif __GNUC__
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wpedantic"
        #endif
        union {
            struct { T x, y; };
            typename Vec2Datas<T>::data data;
        };
        #ifdef __clang__
            #pragma clang diagnostic pop
            #pragma clang diagnostic pop
        #elif __GNUC__
            #pragma GCC diagnostic pop
        #endif

    public:
        constexpr Vec2T(T val = 0) noexcept:
            x(val), y(val) {
        }

        constexpr Vec2T(T newX, T newY) noexcept:
            x(newX), y(newY) {
        }

        constexpr Vec2T(typename Vec2Datas<T>::data newData) noexcept:
            data(newData) {
        }

        constexpr Vec2T(const Vec2T& oth) noexcept:
            data(oth.data) {
        }

    public:
        #ifdef __ARM_NEON
            [[nodiscard]] constexpr operator float32x2_t&() noexcept {
                return data;
            }
        #endif

    public:
        constexpr Vec2T& operator =(const Vec2T& oth) noexcept {
            data = oth.data;
            return *this;
        }

    public:
        [[nodiscard]] constexpr T operator [](const size_t index) const noexcept(false) {
            switch (index) {
                case 0: return x;
                case 1: return y;
                default: throw "Out of range of Vec2";
            }
        }

    public:
        [[nodiscard]] constexpr bool operator ==(const Vec2T& oth) const noexcept {
            return {x == oth.x && y == oth.y};
        }

        [[nodiscard]] constexpr bool operator !=(const Vec2T& oth) const noexcept {
            return {x != oth.x || y != oth.y};
        }

    public:
        [[nodiscard]] inline bool operator <=(const Vec2T& oth) const noexcept {
            return x <= oth.x && y <= oth.y;
        }

        [[nodiscard]] inline bool operator >=(const Vec2T& oth) const noexcept {
            return x >= oth.x && y >= oth.y;
        }

    public:
        [[nodiscard]] constexpr Vec2T operator +(const T val) const noexcept {
            return {x + val, y + val};
        }

        [[nodiscard]] constexpr Vec2T operator -(const T val) const noexcept {
            return {x - val, y - val};
        }

        [[nodiscard]] constexpr Vec2T operator *(const T val) const noexcept {
            return {x * val, y * val};
        }

        [[nodiscard]] constexpr Vec2T operator /(const T val) const noexcept {
            return {x / val, y / val};
        }

    public:
        [[nodiscard]] constexpr Vec2T operator +(const Vec2T& oth) const noexcept {
            return {x + oth.x, y + oth.y};
        }
        
        [[nodiscard]] constexpr Vec2T operator -(const Vec2T& oth) const noexcept {
            return {x - oth.x, y - oth.y};
        }

        [[nodiscard]] constexpr Vec2T operator *(const Vec2T& oth) const noexcept {
            return {x * oth.x, y * oth.y};
        }

        [[nodiscard]] constexpr Vec2T operator /(const Vec2T& oth) const noexcept {
            return {x / oth.x, y / oth.y};
        }

    public:
        constexpr Vec2T& operator +=(const T val) noexcept {
            x += val;
            y += val;
            return *this;
        }

        constexpr Vec2T& operator -=(const T val) noexcept {
            x -= val;
            y -= val;
            return *this;
        }

        constexpr Vec2T& operator *=(const T val) noexcept {
            x *= val;
            y *= val;
            return *this;
        }

        constexpr Vec2T& operator /=(const T val) noexcept {
            x /= val;
            y /= val;
            return *this;
        }

    public:
        constexpr Vec2T& operator +=(const Vec2T& oth) noexcept {
            x += oth.x;
            y += oth.y;
            return *this;
        }

        constexpr Vec2T& operator -=(const Vec2T& oth) noexcept {
            x -= oth.x;
            y -= oth.y;
            return *this;
        }

        constexpr Vec2T& operator *=(const Vec2T& oth) noexcept {
            x *= oth.x;
            y *= oth.y;
            return *this;
        }

        constexpr Vec2T& operator /=(const Vec2T& oth) noexcept {
            x /= oth.x;
            y /= oth.y;
            return *this;
        }

    public:
        [[nodiscard]] constexpr T length() const noexcept {
            return static_cast<T>(sqrt((x * x) + (y * y)));
        }
    };

    using Vec2 = Vec2T<float>;
    using Vec2f = Vec2T<float>;
    using Vec2float = Vec2T<float>;
    using Vec2d = Vec2T<double>;
    using Vec2double = Vec2T<double>;

    using Vec2u = Vec2T<unsigned int>;
    using Vec2ui = Vec2T<unsigned int>;
    using Vec2uint = Vec2T<unsigned int>;
    using Vec2u8 = Vec2T<uint_fast8_t>;
    using Vec2u16 = Vec2T<uint_fast16_t>;
    using Vec2u32 = Vec2T<uint_fast32_t>;
    using Vec2u64 = Vec2T<uint_fast64_t>;

    using Vec2i = Vec2T<int>;
    using Vec2int = Vec2T<int>;
    using Vec2i8 = Vec2T<int_fast8_t>;
    using Vec2i16 = Vec2T<int_fast16_t>;
    using Vec2i32 = Vec2T<int_fast32_t>;
    using Vec2i64 = Vec2T<int_fast64_t>;

    using Vec2b = Vec2T<bool>;
    using Vec2bool = Vec2T<bool>;
}

#endif