#ifndef ZERENGINE_MATHS_VEC3
#define ZERENGINE_MATHS_VEC3

#include <cstdint>
#include <cmath>

#ifdef __ARM_NEON
    #include <arm_neon.h>
#endif

namespace zre {
    template <typename T>
    struct Vec3Datas {
        typedef struct {
            T data[3];
        } data;
    };

    template <typename T>
    struct Vec3T {
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
            struct { T x, y, z; };
            struct { T r, g, b; };
            T ptr[3];
            typename Vec3Datas<T>::data data;
        };
        #ifdef __clang__
            #pragma clang diagnostic pop
            #pragma clang diagnostic pop
        #elif __GNUC__
            #pragma GCC diagnostic pop
        #endif

    public:
        constexpr Vec3T(T val = 0) noexcept:
            x(val), y(val), z(val) {
        }

        constexpr Vec3T(T newX, T newY, T newZ) noexcept:
            x(newX), y(newY), z(newZ) {
        }

        constexpr Vec3T(typename Vec3Datas<T>::data newData) noexcept:
            data(newData) {
        }

        constexpr Vec3T(const Vec3T& oth) noexcept:
            data(oth.data) {
        }

    public:
        constexpr Vec3T& operator =(const Vec3T& oth) noexcept {
            data = oth.data;
            return *this;
        }

    public:
        constexpr T& operator [](const size_t index) noexcept(false) {
            switch (index) {
                case 0: return x;
                case 1: return y;
                case 2: return z;
                default: throw "Out of range of Vec2";
            }
        }

    public:
        constexpr bool operator ==(const Vec3T& oth) const noexcept {
            return
                x == oth.x &&
                y == oth.y &&
                z == oth.z;
        }

        constexpr bool operator !=(const Vec3T& oth) const noexcept {
            return
                x != oth.x ||
                y != oth.y ||
                z != oth.z;
        }

    public:
        constexpr Vec3T operator +(const T val) const noexcept {
            return {x + val, y + val, z + val};
        }

        constexpr Vec3T operator -(const T val) const noexcept {
            return {x - val, y - val, z - val};
        }

        constexpr Vec3T operator *(const T val) const noexcept {
            return {x * val, y * val, z * val};
        }

        constexpr Vec3T operator /(const T val) const noexcept {
            return {x / val, y / val, z / val};
        }

    public:
        constexpr Vec3T operator +(const Vec3T& oth) const noexcept {
            return {x + oth.x, y + oth.y, z + oth.z};
        }
        
        constexpr Vec3T operator -(const Vec3T& oth) const noexcept {
            return {x - oth.x, y - oth.y, z - oth.z};
        }

        constexpr Vec3T operator *(const Vec3T& oth) const noexcept {
            return {x * oth.x, y * oth.y, z * oth.z};
        }

        constexpr Vec3T operator /(const Vec3T& oth) const noexcept {
            return {x / oth.x, y / oth.y, z / oth.z};
        }

    public:
        constexpr Vec3T& operator +=(const T val) noexcept {
            x += val;
            y += val;
            z += val;
            return *this;
        }

        constexpr Vec3T& operator -=(const T val) noexcept {
            x -= val;
            y -= val;
            z -= val;
            return *this;
        }

        constexpr Vec3T& operator *=(const T val) noexcept {
            x *= val;
            y *= val;
            z *= val;
            return *this;
        }

        constexpr Vec3T& operator /=(const T val) noexcept {
            x /= val;
            y /= val;
            z /= val;
            return *this;
        }

    public:
        constexpr Vec3T& operator +=(const Vec3T& oth) noexcept {
            x += oth.x;
            y += oth.y;
            z += oth.z;
            return *this;
        }

        constexpr Vec3T& operator -=(const Vec3T& oth) noexcept {
            x -= oth.x;
            y -= oth.y;
            z -= oth.z;
            return *this;
        }

        constexpr Vec3T& operator *=(const Vec3T& oth) noexcept {
            x *= oth.x;
            y *= oth.y;
            z *= oth.z;
            return *this;
        }

        constexpr Vec3T& operator /=(const Vec3T& oth) noexcept {
            x /= oth.x;
            y /= oth.y;
            z /= oth.z;
            return *this;
        }
    };

    using Vec3 = Vec3T<float>;
    using Vec3f = Vec3T<float>;
    using Vec3float = Vec3T<float>;
    using Vec3d = Vec3T<double>;
    using Vec3double = Vec3T<double>;

    using Vec3u = Vec3T<unsigned int>;
    using Vec3ui = Vec3T<unsigned int>;
    using Vec3uint = Vec3T<unsigned int>;
    using Vec3u8 = Vec3T<uint_fast8_t>;
    using Vec3u16 = Vec3T<uint_fast16_t>;
    using Vec3u32 = Vec3T<uint_fast32_t>;
    using Vec3u64 = Vec3T<uint_fast64_t>;

    using Vec3i = Vec3T<int>;
    using Vec3int = Vec3T<int>;
    using Vec3i8 = Vec3T<int_fast8_t>;
    using Vec3i16 = Vec3T<int_fast16_t>;
    using Vec3i32 = Vec3T<int_fast32_t>;
    using Vec3i64 = Vec3T<int_fast64_t>;

    using Vec3b = Vec3T<bool>;
    using Vec3bool = Vec3T<bool>;
}

#endif