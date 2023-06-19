#ifndef ZERENGINE_MATHS_VEC4
#define ZERENGINE_MATHS_VEC4

#include <cstdint>
#include <cmath>

#ifdef __SSE2__
    #include <immintrin.h>
#elif __ARM_NEON
    #include <arm_neon.h>
#endif

namespace zre {
    template <typename T>
    struct Vec4Datas {
        typedef T data[4];
    };

    #ifdef __SSE2__
        template <>
        struct Vec4Datas<float> {
            typedef __m128 data;
        };

        template <>
        struct Vec4Datas<int32_t> {
            typedef __m128i data;
        };
    #endif

    #ifdef __ARM_NEON
        template <>
        struct Vec4Datas<float> {
            typedef float32x4_t data;
        };

        template <>
        struct Vec4Datas<int32_t> {
            typedef int32x4_t data;
        };
        #ifdef __arm64__
            template <>
            struct Vec4Datas<double> {
                typedef float64x2_t data;
            };
        #endif
    #endif

    #ifdef __AVX2__
        template <>
        struct Vec4Datas<double> {
            typedef __m256d data;
        };
    #endif

    template <typename T>
    struct Vec4T {
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
            struct { T x, y, z, w; };
            struct { T minX, minY, maxX, maxY; };
            struct { T r, g, b, a; };
            T ptr[4];
            typename Vec4Datas<T>::data data;
        };
        #ifdef __clang__
            #pragma clang diagnostic pop
            #pragma clang diagnostic pop
        #elif __GNUC__
            #pragma GCC diagnostic pop
        #endif

    public:
        constexpr Vec4T(T val = 0) noexcept:
            x(val), y(val), z(val), w(val) {
        }

        constexpr Vec4T(T newX, T newY, T newZ, T newW) noexcept:
            x(newX), y(newY), z(newZ), w(newW) {
        }

        constexpr Vec4T(typename Vec4Datas<T>::data newData) noexcept:
            data(newData) {
        }

        constexpr Vec4T(const Vec4T& oth) noexcept:
            data(oth.data) {
        }

    public:
        #ifdef __ARM_NEON
            [[nodiscard]] constexpr operator float32x4_t&() noexcept {
                return data;
            }
        #endif

    public:
        constexpr Vec4T& operator =(const Vec4T& oth) noexcept {
            data = oth.data;
            return *this;
        }

    public:
        [[nodiscard]] constexpr T& operator [](const size_t index) noexcept(false) {
            switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                case 3: return w;
                default: throw "Out of range of Vec4";
            }
        }

    public:
        [[nodiscard]] constexpr bool operator ==(const Vec4T& oth) const noexcept {
            return data == oth.data;
        }

        [[nodiscard]] constexpr bool operator !=(const Vec4T& oth) const noexcept {
            return data != oth.data;
        }

    public:
        [[nodiscard]] constexpr Vec4T operator +(const T val) const noexcept {
            return {x + val, y + val, z + val, w + val};
        }

        [[nodiscard]] constexpr Vec4T operator -(const T val) const noexcept {
            return {x - val, y - val, z - val, w - val};
        }

        [[nodiscard]] constexpr Vec4T operator *(const T val) const noexcept {
            return {x * val, y * val, z * val, w * val};
        }

        [[nodiscard]] constexpr Vec4T operator /(const T val) const noexcept {
            return {x / val, y / val, z / val, w / val};
        }

    public:
        [[nodiscard]] constexpr Vec4T operator +(const Vec4T& oth) const noexcept {
            return {x + oth.x, y + oth.y, z + oth.z, w + oth.w};
        }

        [[nodiscard]] constexpr Vec4T operator -(const Vec4T& oth) const noexcept {
            return {x - oth.x, y - oth.y, z - oth.z, w - oth.w};
        }

        [[nodiscard]] constexpr Vec4T operator *(const Vec4T& oth) const noexcept {
            return {x * oth.x, y * oth.y, z * oth.z, w * oth.w};
        }

        [[nodiscard]] constexpr Vec4T operator /(const Vec4T& oth) const noexcept {
            return {x / oth.x, y / oth.y, z / oth.z, w / oth.w};
        }

    public:
        constexpr Vec4T& operator +=(const T val) noexcept {
            x += val;
            y += val;
            z += val;
            w += val;
            return *this;
        }

        constexpr Vec4T& operator -=(const T val) noexcept {
            x -= val;
            y -= val;
            z -= val;
            w -= val;
            return *this;
        }

        constexpr Vec4T& operator *=(const T val) noexcept {
            x *= val;
            y *= val;
            z *= val;
            w *= val;
            return *this;
        }

        constexpr Vec4T& operator /=(const T val) noexcept {
            x /= val;
            y /= val;
            z /= val;
            w /= val;
            return *this;
        }

    public:
        constexpr Vec4T& operator +=(const Vec4T& oth) noexcept {
            x += oth.x;
            y += oth.y;
            z += oth.z;
            w += oth.w;
            return *this;
        }

        constexpr Vec4T& operator -=(const Vec4T& oth) noexcept {
            x -= oth.x;
            y -= oth.y;
            z -= oth.z;
            w -= oth.w;
            return *this;
        }

        constexpr Vec4T& operator *=(const Vec4T& oth) noexcept {
            x *= oth.x;
            y *= oth.y;
            z *= oth.z;
            w *= oth.w;
            return *this;
        }

        constexpr Vec4T& operator /=(const Vec4T& oth) noexcept {
            x /= oth.x;
            y /= oth.y;
            z /= oth.z;
            w /= oth.w;
        }

    public:
        [[nodiscard]] constexpr T length() const noexcept {
            return static_cast<T>(sqrt((x * x) + (y * y) + (z * z) + (w * w)));
        }
/*
        friend constexpr T length(const Vec4T& vec) noexcept {
            return static_cast<T>(sqrt((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z) + (vec.w * vec.w)));
        }

        friend constexpr T distance(const Vec4T& vecA, const Vec4T& vecB) noexcept {
            return length(vecB - vecA);
        }
*/
        [[nodiscard]] constexpr Vec4T normalize() const noexcept {
            T lgt = length();
            return {
                x / lgt,
                y / lgt,
                z / lgt,
                w / lgt
            };
        }
    };

    using Vec4 = Vec4T<float>;
    using Vec4f = Vec4T<float>;
    using Vec4float = Vec4T<float>;
    using Vec4d = Vec4T<double>;
    using Vec4double = Vec4T<double>;

    using Vec4u = Vec4T<unsigned int>;
    using Vec4ui = Vec4T<unsigned int>;
    using Vec4uint = Vec4T<unsigned int>;
    using Vec4u8 = Vec4T<uint_fast8_t>;
    using Vec4u16 = Vec4T<uint_fast16_t>;
    using Vec4u32 = Vec4T<uint_fast32_t>;
    using Vec4u64 = Vec4T<uint_fast64_t>;

    using Vec4i = Vec4T<int>;
    using Vec4int = Vec4T<int>;
    using Vec4i8 = Vec4T<int_fast8_t>;
    using Vec4i16 = Vec4T<int_fast16_t>;
    using Vec4i32 = Vec4T<int_fast32_t>;
    using Vec4i64 = Vec4T<int_fast64_t>;

    using Vec4b = Vec4T<bool>;
    using Vec4bool = Vec4T<bool>;
}

#endif
