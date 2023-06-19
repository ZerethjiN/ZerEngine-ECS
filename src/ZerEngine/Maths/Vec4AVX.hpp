#ifndef ZERENGINE_MATHS_VEC4_AVX
#define ZERENGINE_MATHS_VEC4_AVX

#include "Vec4.hpp"

#ifdef __AVX2__
    #include <immintrin.h>

    namespace zre {
        // template <>
        // inline bool Vec4T<double>::operator ==(const Vec4T<double>& oth) const noexcept {
        //     return _mm256_movemask_pd(_mm256_cmpeq_pd(data, oth.data));
        // }

        // template <>
        // inline bool Vec4T<double>::operator !=(const Vec4T<double>& oth) const noexcept {
        //     return _mm256_movemask_pd(_mm256_cmpneq_pd(data, oth.data));
        // }

        ////////////////////////////////////////////////////////////////////////////////////////////

        template <>
        inline Vec4T<double> Vec4T<double>::operator +(double val) const noexcept {
            return _mm256_add_pd(data, _mm256_set1_pd(val));
        }

        template <>
        inline Vec4T<double> Vec4T<double>::operator -(double val) const noexcept {
            return _mm256_sub_pd(data, _mm256_set1_pd(val));
        }

        template <>
        inline Vec4T<double> Vec4T<double>::operator *(double val) const noexcept {
            return _mm256_mul_pd(data, _mm256_set1_pd(val));
        }

        template <>
        inline Vec4T<double> Vec4T<double>::operator /(double val) const noexcept {
            return _mm256_div_pd(data, _mm256_set1_pd(val));
        }

        ////////////////////////////////////////////////////////////////////////////////////////////

        template <>
        inline Vec4T<double> Vec4T<double>::operator +(const Vec4T<double>& oth) const noexcept {
            return _mm256_add_pd(data, oth.data);
        }

        template <>
        inline Vec4T<double> Vec4T<double>::operator -(const Vec4T<double>& oth) const noexcept {
            return _mm256_sub_pd(data, oth.data);
        }

        template <>
        inline Vec4T<double> Vec4T<double>::operator *(const Vec4T<double>& oth) const noexcept {
            return _mm256_mul_pd(data, oth.data);
        }

        template <>
        inline Vec4T<double> Vec4T<double>::operator /(const Vec4T<double>& oth) const noexcept {
            return _mm256_div_pd(data, oth.data);
        }

        ////////////////////////////////////////////////////////////////////////////////////////////

        template <>
        inline Vec4T<double>& Vec4T<double>::operator +=(double val) noexcept {
            data = _mm256_add_pd(data, _mm256_set1_pd(val));
            return *this;
        }

        template <>
        inline Vec4T<double>& Vec4T<double>::operator -=(double val) noexcept {
            data = _mm256_sub_pd(data, _mm256_set1_pd(val));
            return *this;
        }

        template <>
        inline Vec4T<double>& Vec4T<double>::operator *=(double val) noexcept {
            data = _mm256_mul_pd(data, _mm256_set1_pd(val));
            return *this;
        }

        template <>
        inline Vec4T<double>& Vec4T<double>::operator /=(double val) noexcept {
            data = _mm256_div_pd(data, _mm256_set1_pd(val));
            return *this;
        }

        ////////////////////////////////////////////////////////////////////////////////////////////

        template <>
        inline Vec4T<double>& Vec4T<double>::operator +=(const Vec4T<double>& oth) noexcept {
            data = _mm256_add_pd(data, oth.data);
            return *this;
        }

        template <>
        inline Vec4T<double>& Vec4T<double>::operator -=(const Vec4T<double>& oth) noexcept {
            data = _mm256_sub_pd(data, oth.data);
            return *this;
        }

        template <>
        inline Vec4T<double>& Vec4T<double>::operator *=(const Vec4T<double>& oth) noexcept {
            data = _mm256_mul_pd(data, oth.data);
            return *this;
        }

        template <>
        inline Vec4T<double>& Vec4T<double>::operator /=(const Vec4T<double>& oth) noexcept {
            data = _mm256_div_pd(data, oth.data);
            return *this;
        }
    }
#endif

#endif