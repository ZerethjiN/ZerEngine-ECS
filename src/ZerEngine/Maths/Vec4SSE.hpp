#ifndef ZERENGINE_MATHS_VEC4_SSE
#define ZERENGINE_MATHS_VEC4_SSE

#include "Vec4.hpp"
#include <cstdint>

#ifdef __SSE2__
    #include <immintrin.h>

    namespace zre {
        template <>
        inline bool Vec4T<float>::operator ==(const Vec4T<float>& oth) const noexcept {
            return _mm_movemask_ps(_mm_cmpeq_ps(data, oth.data));
        }

        template <>
        inline bool Vec4T<float>::operator !=(const Vec4T<float>& oth) const noexcept {
            return _mm_movemask_ps(_mm_cmpneq_ps(data, oth.data));
        }

        ////////////////////////////////////////////////////////////////////////////////////////////

        //// Add ////
        template <>
        inline Vec4T<float> Vec4T<float>::operator +(float val) const noexcept {
            return _mm_add_ps(data, _mm_set1_ps(val));
        }

        template <>
        inline Vec4T<int32_t> Vec4T<int32_t>::operator +(int32_t val) const noexcept {
            return _mm_add_epi32(data, _mm_set1_epi32(val));
        }

        //// Sub ////
        template <>
        inline Vec4T<float> Vec4T<float>::operator -(float val) const noexcept {
            return _mm_sub_ps(data, _mm_set1_ps(val));
        }

        template <>
        inline Vec4T<int32_t> Vec4T<int32_t>::operator -(int32_t val) const noexcept {
            return _mm_sub_epi32(data, _mm_set1_epi32(val));
        }

        //// Mul ////
        template <>
        inline Vec4T<float> Vec4T<float>::operator *(float val) const noexcept {
            return _mm_mul_ps(data, _mm_set1_ps(val));
        }

        template <>
        inline Vec4T<int32_t> Vec4T<int32_t>::operator *(int32_t val) const noexcept {
            return _mm_mul_epi32(data, _mm_set1_epi32(val));
        }

        //// Div ////
        template <>
        inline Vec4T<float> Vec4T<float>::operator /(float val) const noexcept {
            return _mm_div_ps(data, _mm_set1_ps(val));
        }
/*
        template <>
        inline Vec4T<int32_t> Vec4T<int32_t>::operator /(int32_t val) const noexcept {
            return _mm_div_epi32(data, _mm_set1_epi32(val));
        }
*/
        ////////////////////////////////////////////////////////////////////////////////////////////

        //// Add ////
        template <>
        inline Vec4T<float> Vec4T<float>::operator +(const Vec4T<float>& oth) const noexcept {
            return _mm_add_ps(data, oth.data);
        }

        template <>
        inline Vec4T<int32_t> Vec4T<int32_t>::operator +(const Vec4T<int32_t>& oth) const noexcept {
            return _mm_add_epi32(data, oth.data);
        }

        //// Sub ////
        template <>
        inline Vec4T<float> Vec4T<float>::operator -(const Vec4T<float>& oth) const noexcept {
            return _mm_sub_ps(data, oth.data);
        }

        template <>
        inline Vec4T<int32_t> Vec4T<int32_t>::operator -(const Vec4T<int32_t>& oth) const noexcept {
            return _mm_sub_epi32(data, oth.data);
        }

        //// Mul ////
        template <>
        inline Vec4T<float> Vec4T<float>::operator *(const Vec4T<float>& oth) const noexcept {
            return _mm_mul_ps(data, oth.data);
        }

        template <>
        inline Vec4T<int32_t> Vec4T<int32_t>::operator *(const Vec4T<int32_t>& oth) const noexcept {
            return _mm_mul_epi32(data, oth.data);
        }

        //// Div ////
        template <>
        inline Vec4T<float> Vec4T<float>::operator /(const Vec4T<float>& oth) const noexcept {
            return _mm_div_ps(data, oth.data);
        }
/*
        template <>
        inline Vec4T<int32_t> Vec4T<int32_t>::operator /(const Vec4T<int32_t>& oth) const noexcept {
            return _mm_div_epi32(data, oth.data);
        }
*/
        ////////////////////////////////////////////////////////////////////////////////////////////

        //// Add ////
        template <>
        inline Vec4T<float>& Vec4T<float>::operator +=(float val) noexcept {
            data = _mm_add_ps(data, _mm_set1_ps(val));
            return *this;
        }

        template <>
        inline Vec4T<int32_t>& Vec4T<int32_t>::operator +=(int32_t val) noexcept {
            data = _mm_add_epi32(data, _mm_set1_epi32(val));
            return *this;
        }

        //// Sub ////
        template <>
        inline Vec4T<float>& Vec4T<float>::operator -=(float val) noexcept {
            data = _mm_sub_ps(data, _mm_set1_ps(val));
            return *this;
        }

        template <>
        inline Vec4T<int32_t>& Vec4T<int32_t>::operator -=(int32_t val) noexcept {
            data = _mm_sub_epi32(data, _mm_set1_epi32(val));
            return *this;
        }

        //// Mul ////
        template <>
        inline Vec4T<float>& Vec4T<float>::operator *=(float val) noexcept {
            data = _mm_mul_ps(data, _mm_set1_ps(val));
            return *this;
        }

        template <>
        inline Vec4T<int32_t>& Vec4T<int32_t>::operator *=(int32_t val) noexcept {
            data = _mm_mul_epi32(data, _mm_set1_epi32(val));
            return *this;
        }

        //// Div ////
        template <>
        inline Vec4T<float>& Vec4T<float>::operator /=(float val) noexcept {
            data = _mm_div_ps(data, _mm_set1_ps(val));
            return *this;
        }
/*
        template <>
        inline Vec4T<int32_t>& Vec4T<int32_t>::operator /=(int32_t val) noexcept {
            data = _mm_div_epi32(data, _mm_set1_epi32(val));
            return *this;
        }
*/
        ////////////////////////////////////////////////////////////////////////////////////////////

        //// Add ////
        template <>
        inline Vec4T<float>& Vec4T<float>::operator +=(const Vec4T<float>& oth) noexcept {
            data = _mm_add_ps(data, oth.data);
            return *this;
        }

        template <>
        inline Vec4T<int32_t>& Vec4T<int32_t>::operator +=(const Vec4T<int32_t>& oth) noexcept {
            data = _mm_add_epi32(data, oth.data);
            return *this;
        }

        //// Sub ////
        template <>
        inline Vec4T<float>& Vec4T<float>::operator -=(const Vec4T<float>& oth) noexcept {
            data = _mm_sub_ps(data, oth.data);
            return *this;
        }

        template <>
        inline Vec4T<int32_t>& Vec4T<int32_t>::operator -=(const Vec4T<int32_t>& oth) noexcept {
            data = _mm_sub_epi32(data, oth.data);
            return *this;
        }

        //// Mul ////
        template <>
        inline Vec4T<float>& Vec4T<float>::operator *=(const Vec4T<float>& oth) noexcept {
            data = _mm_mul_ps(data, oth.data);
            return *this;
        }

        template <>
        inline Vec4T<int32_t>& Vec4T<int32_t>::operator *=(const Vec4T<int32_t>& oth) noexcept {
            data = _mm_mul_epi32(data, oth.data);
            return *this;
        }

        //// Div ////
        template <>
        inline Vec4T<float>& Vec4T<float>::operator /=(const Vec4T<float>& oth) noexcept {
            data = _mm_div_ps(data, oth.data);
            return *this;
        }
/*
        template <>
        inline Vec4T<int32_t>& Vec4T<int32_t>::operator /=(const Vec4T<int32_t>& oth) noexcept {
            data = _mm_div_epi32(data, oth.data);
            return *this;
        }
*/
        ////////////////////////////////////////////////////////////////////////////////////////////

        template <>
        inline Vec4T<float> Vec4T<float>::normalize() const noexcept {
            return _mm_div_ps(data, _mm_set1_ps(length()));
        }
    }
#endif

#endif