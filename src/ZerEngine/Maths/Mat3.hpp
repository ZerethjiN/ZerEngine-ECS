#ifndef ZERENGINE_MATHS_MAT3_HPP
#define ZERENGINE_MATHS_MAT3_HPP

#include <cstdint>
#include "Vec2.hpp"
#include "Vec3.hpp"

#ifdef __ARM_NEON
    #include <arm_neon.h>
    #include "Vec2NEON.hpp"
#endif

namespace zre {
    template <typename T>
    struct Mat3Datas {
        typedef T data[9];
    };

    template <typename T>
    struct Mat3T {
    public:
        union {
            zre::Vec3T<T> vecs[3];
            typename Mat3Datas<T>::data data;
        };

    public:
        constexpr Mat3T(T val = 1) noexcept:
            vecs {
                {val, 0, 0},
                {0, val, 0},
                {0, 0, val},
            } {
        }

        constexpr Mat3T(const Vec3T<T>& vec0, const Vec3T<T>& vec1, const Vec3T<T>& vec2) noexcept:
            vecs{vec0, vec1, vec2} {
        }

        constexpr Mat3T(typename Mat3Datas<T>::data newData) noexcept:
            data(newData) {
        }

        constexpr Mat3T(const Mat3T& oth) noexcept:
            vecs{oth.vecs[0], oth.vecs[1], oth.vecs[2]} {
        }

    public:
        constexpr Mat3T& operator =(const Mat3T& oth) noexcept {
            vecs[0] = oth.vecs[0];
            vecs[1] = oth.vecs[1];
            vecs[2] = oth.vecs[2];
            return *this;
        }

    public:
        constexpr Vec3T<T>& operator [](const size_t index) noexcept(false) {
            if (index >= 3)
                throw "Out of range of Mat3";
            return vecs[index];
        }

    public:
        constexpr Mat3T operator *(const Mat3T& oth) const noexcept {
            return {
                vecs[0] * oth.vecs[0],
                vecs[1] * oth.vecs[1],
                vecs[2] * oth.vecs[2]
            };
        }

    public:
        constexpr Mat3T& operator *=(const Mat3T& oth) const noexcept {
            vecs[0] *= oth.vecs[0];
            vecs[1] *= oth.vecs[1];
            vecs[2] *= oth.vecs[2];
            return *this;
        }

    public:
        static constexpr Mat3T identity() noexcept {
            return {
                {1, 0, 0},
                {0, 1, 0},
                {0, 0, 1}
            };
        }

        static constexpr Mat3T zero() noexcept {
            return {
                {0, 0, 0},
                {0, 0, 0},
                {0, 0, 0}
            };
        }

    public:
        constexpr Mat3T& translate(const Vec2T<T>& vec) noexcept {
            vecs[2][0] += vec.x;
            vecs[2][1] += vec.y;
            return *this;
        }

        constexpr Mat3T& rotate(T angle) noexcept {
            const T c = cos(angle);
            const T s = sin(angle);
            vecs[0][0] *= c;
            vecs[1][0] *= s;
            vecs[0][1] *= -s;
            vecs[1][1] *= c;
            return *this;
        }

        constexpr Mat3T& scale(const Vec2T<T>& vec) noexcept {
            vecs[0][0] *= vec.x;
            vecs[1][1] *= vec.y;
            return *this;
        }
    };

    using Mat3 = Mat3T<float>;
}

#endif /* ZERENGINE_MATHS_MAT3_HPP */