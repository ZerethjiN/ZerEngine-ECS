#ifndef ZERENGINE_MATHS_VEC4_NEON
#define ZERENGINE_MATHS_VEC4_NEON

#include "Vec4.hpp"
#include <cstdint>

#ifdef __ARM_NEON
    #include <arm_neon.h>

    namespace zre {
/*
        //// Equal ////
        template <>
        constexpr bool Vec4T<float>::operator ==(const Vec4T<float>& oth) const noexcept {
            return vceqq_f32(data, oth.data);
        }
*/
        ////////////////////////////////////////////////////////////////////////////////////////////

        //// Add ////
        template <>
        [[nodiscard]] constexpr Vec4T<float> Vec4T<float>::operator +(float val) const noexcept {
            return vaddq_f32(data, vdupq_n_f32(val));
        }

        template <>
        [[nodiscard]] constexpr Vec4T<int32_t> Vec4T<int32_t>::operator +(int32_t val) const noexcept {
            return vaddq_s32(data, vdupq_n_s32(val));
        }

        //// Sub ////
        template <>
        [[nodiscard]] constexpr Vec4T<float> Vec4T<float>::operator -(float val) const noexcept {
            return vsubq_f32(data, vdupq_n_f32(val));
        }

        template <>
        [[nodiscard]] constexpr Vec4T<int32_t> Vec4T<int32_t>::operator -(int32_t val) const noexcept {
            return vsubq_s32(data, vdupq_n_s32(val));
        }

        //// Mul ////
        template <>
        [[nodiscard]] constexpr Vec4T<float> Vec4T<float>::operator *(float val) const noexcept {
            return vmulq_f32(data, vdupq_n_f32(val));
        }

        template <>
        [[nodiscard]] constexpr Vec4T<int32_t> Vec4T<int32_t>::operator *(int32_t val) const noexcept {
            return vmulq_s32(data, vdupq_n_s32(val));
        }

        //// Div ////
        #ifdef __arm64__
            template <>
            [[nodiscard]] constexpr Vec4T<float> Vec4T<float>::operator /(float val) const noexcept {
                return vdivq_f32(data, vdupq_n_f32(val));
            }
        #endif

        ////////////////////////////////////////////////////////////////////////////////////////////

        //// Add ////
        template <>
        [[nodiscard]] constexpr Vec4T<float> Vec4T<float>::operator +(const Vec4T<float>& oth) const noexcept {
            return vaddq_f32(data, oth.data);
        }

        template <>
        [[nodiscard]] constexpr Vec4T<int32_t> Vec4T<int32_t>::operator +(const Vec4T<int32_t>& oth) const noexcept {
            return vaddq_s32(data, oth.data);
        }

        //// Sub ////
        template <>
        [[nodiscard]] constexpr Vec4T<float> Vec4T<float>::operator -(const Vec4T<float>& oth) const noexcept {
            return vsubq_f32(data, oth.data);
        }

        template <>
        [[nodiscard]] constexpr Vec4T<int32_t> Vec4T<int32_t>::operator -(const Vec4T<int32_t>& oth) const noexcept {
            return vsubq_s32(data, oth.data);
        }

        //// Mul ////
        template <>
        [[nodiscard]] constexpr Vec4T<float> Vec4T<float>::operator *(const Vec4T<float>& oth) const noexcept {
            return vmulq_f32(data, oth.data);
        }

        template <>
        [[nodiscard]] constexpr Vec4T<int32_t> Vec4T<int32_t>::operator *(const Vec4T<int32_t>& oth) const noexcept {
            return vmulq_s32(data, oth.data);
        }

        //// Div ////
        #ifdef __arm64__
            template <>
            [[nodiscard]] constexpr Vec4T<float> Vec4T<float>::operator /(const Vec4T<float>& oth) const noexcept {
                return vdivq_f32(data, oth.data);
            }
        #endif

        ////////////////////////////////////////////////////////////////////////////////////////////

        //// Add ////
        template <>
        constexpr Vec4T<float>& Vec4T<float>::operator +=(float val) noexcept {
            data = vaddq_f32(data, vdupq_n_f32(val));
            return *this;
        }

        template <>
        constexpr Vec4T<int32_t>& Vec4T<int32_t>::operator +=(int32_t val) noexcept {
            data = vaddq_s32(data, vdupq_n_s32(val));
            return *this;
        }

        //// Sub ////
        template <>
        constexpr Vec4T<float>& Vec4T<float>::operator -=(float val) noexcept {
            data = vsubq_f32(data, vdupq_n_f32(val));
            return *this;
        }

        template <>
        constexpr Vec4T<int32_t>& Vec4T<int32_t>::operator -=(int32_t val) noexcept {
            data = vsubq_s32(data, vdupq_n_s32(val));
            return *this;
        }

        //// Mul ////
        template <>
        constexpr Vec4T<float>& Vec4T<float>::operator *=(float val) noexcept {
            data = vmulq_f32(data, vdupq_n_f32(val));
            return *this;
        }

        template <>
        constexpr Vec4T<int32_t>& Vec4T<int32_t>::operator *=(int32_t val) noexcept {
            data = vmulq_s32(data, vdupq_n_s32(val));
            return *this;
        }

        //// Div ////
        #ifdef __arm64__
            template <>
            constexpr Vec4T<float>& Vec4T<float>::operator /=(float val) noexcept {
                data = vdivq_f32(data, vdupq_n_f32(val));
                return *this;
            }
        #endif

        ////////////////////////////////////////////////////////////////////////////////////////////

        //// Add ////
        template <>
        constexpr Vec4T<float>& Vec4T<float>::operator +=(const Vec4T<float>& oth) noexcept {
            data = vaddq_f32(data, oth.data);
            return *this;
        }

        template <>
        constexpr Vec4T<int32_t>& Vec4T<int32_t>::operator +=(const Vec4T<int32_t>& oth) noexcept {
            data = vaddq_s32(data, oth.data);
            return *this;
        }

        //// Sub ////
        template <>
        constexpr Vec4T<float>& Vec4T<float>::operator -=(const Vec4T<float>& oth) noexcept {
            data = vsubq_f32(data, oth.data);
            return *this;
        }

        template <>
        constexpr Vec4T<int32_t>& Vec4T<int32_t>::operator -=(const Vec4T<int32_t>& oth) noexcept {
            data = vsubq_s32(data, oth.data);
            return *this;
        }

        //// Mul ////
        template <>
        constexpr Vec4T<float>& Vec4T<float>::operator *=(const Vec4T<float>& oth) noexcept {
            data = vmulq_f32(data, oth.data);
            return *this;
        }

        template <>
        constexpr Vec4T<int32_t>& Vec4T<int32_t>::operator *=(const Vec4T<int32_t>& oth) noexcept {
            data = vmulq_s32(data, oth.data);
            return *this;
        }

        //// Div ////
        #ifdef __arm64__
            template <>
            constexpr Vec4T<float>& Vec4T<float>::operator /=(const Vec4T<float>& oth) noexcept {
                data = vdivq_f32(data, oth.data);
                return *this;
            }
        #endif

        ////////////////////////////////////////////////////////////////////////////////////////////
        #ifdef __arm64__
            template <>
            [[nodiscard]] constexpr float Vec4T<float>::length() const noexcept {
                return sqrt(vaddvq_f32(vmulq_f32(data, data)));
            }
/*
            constexpr float length(const Vec4T<float>& vec) noexcept {
                return sqrt(vaddvq_f32(vmulq_f32(vec.data, vec.data)));
            }
*/
            template <>
            [[nodiscard]] constexpr Vec4T<float> Vec4T<float>::normalize() const noexcept {
                return vdivq_f32(data, vdupq_n_f32(length()));
            }
        #endif
    }
#endif

#endif
