#ifndef ZERENGINE_MATHS_MAT4_HPP
#define ZERENGINE_MATHS_MAT4_HPP

#include <cstdint>
#include "Vec3.hpp"
#include "Vec4.hpp"

#ifdef __SSE2__
    #include <immintrin.h>
#endif

namespace zre {
    template <typename T>
    struct Mat4Datas {
        typedef T data[16];
    };

    #ifdef __AVX512F__
        template <>
        struct Mat4Datas<float> {
            typedef __m512 data;
        };
    #elif __AVX__
        template <>
        struct Mat4Datas<float> {
            typedef __m256 data[2];
        };
    #endif

    template <typename T>
    struct Mat4T {
    public:
        union {
            zre::Vec4T<T> vecs[4];
            typename Mat4Datas<T>::data data;
        };

    public:
        constexpr Mat4T(T val = 1) noexcept:
            vecs {
                {val, 0, 0, 0},
                {0, val, 0, 0},
                {0, 0, val, 0},
                {0, 0, 0, val}
            } {
        }

        constexpr Mat4T(const Vec4T<T>& vec0, const Vec4T<T>& vec1, const Vec4T<T>& vec2, const Vec4T<T>& vec3) noexcept:
            vecs{vec0, vec1, vec2, vec3} {
        }

        constexpr Mat4T(typename Mat4Datas<T>::data newData) noexcept:
            data(newData) {
        }

        constexpr Mat4T(const Mat4T& oth) noexcept:
            vecs{oth.vecs[0], oth.vecs[1], oth.vecs[2], oth.vecs[3]} {
        }

    public:
        constexpr Mat4T& operator =(const Mat4T& oth) noexcept {
            vecs[0] = oth.vecs[0];
            vecs[1] = oth.vecs[1];
            vecs[2] = oth.vecs[2];
            vecs[3] = oth.vecs[3];
            return *this;
        }

    public:
        constexpr Vec4T<T>& operator [](const size_t index) noexcept(false) {
            if (index >= 4)
                throw "Out of range of Mat4";
            return vecs[index];
        }

    public:
        constexpr bool operator ==(const Mat4T& oth) const noexcept {
            return
                vecs[0] == oth.vecs[0] &&
                vecs[1] == oth.vecs[1] &&
                vecs[2] == oth.vecs[2] &&
                vecs[3] == oth.vecs[3];
        }

        constexpr bool operator !=(const Mat4T& oth) const noexcept {
            return
                vecs[0] != oth.vecs[0] ||
                vecs[1] != oth.vecs[1] ||
                vecs[2] != oth.vecs[2] ||
                vecs[3] != oth.vecs[3];
        }

    public:
        constexpr Mat4T operator *(const Mat4T& oth) const noexcept {
            return {
                vecs[0] * oth.vecs[0],
                vecs[1] * oth.vecs[1],
                vecs[2] * oth.vecs[2],
                vecs[3] * oth.vecs[3]
            };
        }

    public:
        constexpr Mat4T& operator *=(const Mat4T& oth) noexcept {
            vecs[0] *= oth.vecs[0];
            vecs[1] *= oth.vecs[1];
            vecs[2] *= oth.vecs[2];
            vecs[3] *= oth.vecs[3];
            return *this;
        }

        constexpr Mat4T& operator *=(const T val) noexcept {
            vecs[0] *= val;
            vecs[1] *= val;
            vecs[2] *= val;
            vecs[3] *= val;
            return *this;
        }

    public:
        static constexpr Mat4T identity() noexcept {
            return {
                {1, 0, 0, 0},
                {0, 1, 0, 0},
                {0, 0, 1, 0},
                {0, 0, 0, 1}
            };
        }

        static constexpr Mat4T zero() noexcept {
            return {
                {0, 0, 0, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0},
                {0, 0, 0, 0}
            };
        }

    public:
        constexpr Mat4T& translate(const Vec3T<T>& vec) noexcept {
            vecs[0][3] += vec.x;
            vecs[1][3] += vec.y;
            vecs[2][3] += vec.z;
            return *this;
        }

        constexpr Mat4T& scale(const Vec3T<T>& vec) noexcept {
            vecs[0][0] *= vec.x;
            vecs[1][1] *= vec.y;
            vecs[2][2] *= vec.z;
            return *this;
        }

        constexpr Mat4T& inverse() noexcept {
            T coef00 = vecs[2][2] * vecs[3][3] - vecs[3][2] * vecs[2][3];
            T coef02 = vecs[1][2] * vecs[3][3] - vecs[3][2] * vecs[1][3];
            T coef03 = vecs[1][2] * vecs[2][3] - vecs[2][2] * vecs[1][3];

            T coef04 = vecs[2][1] * vecs[3][3] - vecs[3][1] * vecs[2][3];
            T coef06 = vecs[1][1] * vecs[3][3] - vecs[3][1] * vecs[1][3];
            T coef07 = vecs[1][1] * vecs[2][3] - vecs[2][1] * vecs[1][3];

            T coef08 = vecs[2][1] * vecs[3][2] - vecs[3][1] * vecs[2][2];
            T coef10 = vecs[1][1] * vecs[3][2] - vecs[3][1] * vecs[1][2];
            T coef11 = vecs[1][1] * vecs[2][2] - vecs[2][1] * vecs[1][2];

            T coef12 = vecs[2][0] * vecs[3][3] - vecs[3][0] * vecs[2][3];
            T coef14 = vecs[1][0] * vecs[3][3] - vecs[3][0] * vecs[1][3];
            T coef15 = vecs[1][0] * vecs[2][3] - vecs[2][0] * vecs[1][3];

            T coef16 = vecs[2][0] * vecs[3][2] - vecs[3][0] * vecs[2][2];
            T coef18 = vecs[1][0] * vecs[3][2] - vecs[3][0] * vecs[1][2];
            T coef19 = vecs[1][0] * vecs[2][2] - vecs[2][0] * vecs[1][2];

            T coef20 = vecs[2][0] * vecs[3][1] - vecs[3][0] * vecs[2][1];
            T coef22 = vecs[1][0] * vecs[3][1] - vecs[3][0] * vecs[1][1];
            T coef23 = vecs[1][0] * vecs[2][1] - vecs[2][0] * vecs[1][1];

            Vec4T<T> fac0(coef00, coef00, coef02, coef03);
            Vec4T<T> fac1(coef04, coef04, coef06, coef07);
            Vec4T<T> fac2(coef08, coef08, coef10, coef11);
            Vec4T<T> fac3(coef12, coef12, coef14, coef15);
            Vec4T<T> fac4(coef16, coef16, coef18, coef19);
            Vec4T<T> fac5(coef20, coef20, coef22, coef23);

            Vec4T<T> vec0(vecs[1][0], vecs[0][0], vecs[0][0], vecs[0][0]);
            Vec4T<T> vec1(vecs[1][1], vecs[0][1], vecs[0][1], vecs[0][1]);
            Vec4T<T> vec2(vecs[1][2], vecs[0][2], vecs[0][2], vecs[0][2]);
            Vec4T<T> vec3(vecs[1][3], vecs[0][3], vecs[0][3], vecs[0][3]);

            Vec4T<T> inv0(vec1 * fac0 - vec2 * fac1 + vec3 * fac2);
            Vec4T<T> inv1(vec0 * fac0 - vec2 * fac3 + vec3 * fac4);
            Vec4T<T> inv2(vec0 * fac1 - vec1 * fac3 + vec3 * fac5);
            Vec4T<T> inv3(vec0 * fac2 - vec1 * fac4 + vec2 * fac5);

            Vec4T<T> signA(+1, -1, +1, -1);
            Vec4T<T> signB(-1, +1, -1, +1);
            Mat4T<T> inv(inv0 * signA, inv1 * signB, inv2 * signA, inv3 * signB);

            Vec4T<T> row0(inv[0][0], inv[1][0], inv[2][0], inv[3][0]);

            Vec4T<T> dot0(vecs[0] * row0);
            T dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);

            T oneOverDeterminant = static_cast<T>(1) / dot1;

            inv *= oneOverDeterminant;
            vecs[0] = inv.vecs[0];
            vecs[1] = inv.vecs[1];
            vecs[2] = inv.vecs[2];
            vecs[3] = inv.vecs[3];
            return *this;
        }
/*
        constexpr friend Mat4T inverse(Mat4T<T>& oth) noexcept {
            T Coef00 = oth[2][2] * oth[3][3] - oth[3][2] * oth[2][3];
            T Coef02 = oth[1][2] * oth[3][3] - oth[3][2] * oth[1][3];
            T Coef03 = oth[1][2] * oth[2][3] - oth[2][2] * oth[1][3];

            T Coef04 = oth[2][1] * oth[3][3] - oth[3][1] * oth[2][3];
            T Coef06 = oth[1][1] * oth[3][3] - oth[3][1] * oth[1][3];
            T Coef07 = oth[1][1] * oth[2][3] - oth[2][1] * oth[1][3];

            T Coef08 = oth[2][1] * oth[3][2] - oth[3][1] * oth[2][2];
            T Coef10 = oth[1][1] * oth[3][2] - oth[3][1] * oth[1][2];
            T Coef11 = oth[1][1] * oth[2][2] - oth[2][1] * oth[1][2];

            T Coef12 = oth[2][0] * oth[3][3] - oth[3][0] * oth[2][3];
            T Coef14 = oth[1][0] * oth[3][3] - oth[3][0] * oth[1][3];
            T Coef15 = oth[1][0] * oth[2][3] - oth[2][0] * oth[1][3];

            T Coef16 = oth[2][0] * oth[3][2] - oth[3][0] * oth[2][2];
            T Coef18 = oth[1][0] * oth[3][2] - oth[3][0] * oth[1][2];
            T Coef19 = oth[1][0] * oth[2][2] - oth[2][0] * oth[1][2];

            T Coef20 = oth[2][0] * oth[3][1] - oth[3][0] * oth[2][1];
            T Coef22 = oth[1][0] * oth[3][1] - oth[3][0] * oth[1][1];
            T Coef23 = oth[1][0] * oth[2][1] - oth[2][0] * oth[1][1];

            Vec4T<T> Fac0(Coef00, Coef00, Coef02, Coef03);
            Vec4T<T> Fac1(Coef04, Coef04, Coef06, Coef07);
            Vec4T<T> Fac2(Coef08, Coef08, Coef10, Coef11);
            Vec4T<T> Fac3(Coef12, Coef12, Coef14, Coef15);
            Vec4T<T> Fac4(Coef16, Coef16, Coef18, Coef19);
            Vec4T<T> Fac5(Coef20, Coef20, Coef22, Coef23);

            Vec4T<T> Vec0(oth[1][0], oth[0][0], oth[0][0], oth[0][0]);
            Vec4T<T> Vec1(oth[1][1], oth[0][1], oth[0][1], oth[0][1]);
            Vec4T<T> Vec2(oth[1][2], oth[0][2], oth[0][2], oth[0][2]);
            Vec4T<T> Vec3(oth[1][3], oth[0][3], oth[0][3], oth[0][3]);

            Vec4T<T> Inv0(Vec1 * Fac0 - Vec2 * Fac1 + Vec3 * Fac2);
            Vec4T<T> Inv1(Vec0 * Fac0 - Vec2 * Fac3 + Vec3 * Fac4);
            Vec4T<T> Inv2(Vec0 * Fac1 - Vec1 * Fac3 + Vec3 * Fac5);
            Vec4T<T> Inv3(Vec0 * Fac2 - Vec1 * Fac4 + Vec2 * Fac5);

            Vec4T<T> SignA(+1, -1, +1, -1);
            Vec4T<T> SignB(-1, +1, -1, +1);
            Mat4T<T> Inverse(Inv0 * SignA, Inv1 * SignB, Inv2 * SignA, Inv3 * SignB);

            Vec4T<T> Row0(Inverse[0][0], Inverse[1][0], Inverse[2][0], Inverse[3][0]);

            Vec4T<T> Dot0(oth[0] * Row0);
            T Dot1 = (Dot0.x + Dot0.y) + (Dot0.z + Dot0.w);

            T OneOverDeterminant = static_cast<T>(1) / Dot1;

            return Inverse * OneOverDeterminant;
        }
*/
    };

    using Mat4 = Mat4T<float>;
}
/*
        #ifdef __AVX512F__
            template <>
            inline Mat4T<float>& Mat4T<float>::operator +=(const Mat4T<float>& oth) noexcept {
                data = _mm512_add_ps(data, oth.data);
                return *this;
            }
        #elif __AVX__
            template <>
            inline Mat4T<float>& Mat4T<float>::operator +=(const Mat4T<float>& oth) noexcept {
                data[0] = _mm256_add_ps(data[0], oth.data[0]);
                data[1] = _mm256_add_ps(data[1], oth.data[1]);
                return *this;
            }
        #endif
*/
#endif /** ZERENGINE_MATHS_MAT4_HPP */