#ifndef ZERENGINE_MATHS_VEC2_NEON
#define ZERENGINE_MATHS_VEC2_NEON

#include "Vec2.hpp"
#include <cstdint>

#ifdef __ARM_NEON
    #include <arm_neon.h>

    namespace zre {
        //// Greater Than or Equal To ////
/*
        template <>
        inline bool Vec2T<float>::operator <=(const Vec2T<float>& oth) const noexcept {
            return vcge_f32(data, oth.data)[0];
        }

        //// Less Than or Equal To ////
        template <>
        inline bool Vec2T<float>::operator >=(const Vec2T<float>& oth) const noexcept {
            return vcle_f32(data, oth.data)[0];
        }
*/
        ////////////////////////////////////////////////////////////////////////////////////////////

        //// Add ////
        template <>
        constexpr Vec2T<float> Vec2T<float>::operator +(float val) const noexcept {
            return vadd_f32(data, vdup_n_f32(val));
        }

        template <>
        constexpr Vec2T<int32_t> Vec2T<int32_t>::operator +(int32_t val) const noexcept {
            return vadd_s32(data, vdup_n_s32(val));
        }

        //// Sub ////
        template <>
        constexpr Vec2T<float> Vec2T<float>::operator -(float val) const noexcept {
            return vsub_f32(data, vdup_n_f32(val));
        }

        template <>
        constexpr Vec2T<int32_t> Vec2T<int32_t>::operator -(int32_t val) const noexcept {
            return vsub_s32(data, vdup_n_s32(val));
        }

        //// Mul ////
        template <>
        constexpr Vec2T<float> Vec2T<float>::operator *(float val) const noexcept {
            return vmul_f32(data, vdup_n_f32(val));
        }

        template <>
        constexpr Vec2T<int32_t> Vec2T<int32_t>::operator *(int32_t val) const noexcept {
            return vmul_s32(data, vdup_n_s32(val));
        }

        //// Div ////
        #ifdef __arm64__
            template <>
            constexpr Vec2T<float> Vec2T<float>::operator /(float val) const noexcept {
                return vdiv_f32(data, vdup_n_f32(val));
            }
        #endif

        ////////////////////////////////////////////////////////////////////////////////////////////

        //// Add ////
        template <>
        constexpr Vec2T<float> Vec2T<float>::operator +(const Vec2T<float>& oth) const noexcept {
            return vadd_f32(data, oth.data);
        }

        template <>
        constexpr Vec2T<int32_t> Vec2T<int32_t>::operator +(const Vec2T<int32_t>& oth) const noexcept {
            return vadd_s32(data, oth.data);
        }

        //// Sub ////
        template <>
        constexpr Vec2T<float> Vec2T<float>::operator -(const Vec2T<float>& oth) const noexcept {
            return vsub_f32(data, oth.data);
        }

        template <>
        constexpr Vec2T<int32_t> Vec2T<int32_t>::operator -(const Vec2T<int32_t>& oth) const noexcept {
            return vsub_s32(data, oth.data);
        }

        //// Mul ////
        template <>
        constexpr Vec2T<float> Vec2T<float>::operator *(const Vec2T<float>& oth) const noexcept {
            return vmul_f32(data, oth.data);
        }

        template <>
        constexpr Vec2T<int32_t> Vec2T<int32_t>::operator *(const Vec2T<int32_t>& oth) const noexcept {
            return vmul_s32(data, oth.data);
        }

        //// Div ////
        #ifdef __arm64__
            template <>
            constexpr Vec2T<float> Vec2T<float>::operator /(const Vec2T<float>& oth) const noexcept {
                return vdiv_f32(data, oth.data);
            }
        #endif

        ////////////////////////////////////////////////////////////////////////////////////////////

        //// Add ////
        template <>
        constexpr Vec2T<float>& Vec2T<float>::operator +=(float val) noexcept {
            data = vadd_f32(data, vdup_n_f32(val));
            return *this;
        }

        template <>
        constexpr Vec2T<int32_t>& Vec2T<int32_t>::operator +=(int32_t val) noexcept {
            data = vadd_s32(data, vdup_n_s32(val));
            return *this;
        }

        //// Sub ////
        template <>
        constexpr Vec2T<float>& Vec2T<float>::operator -=(float val) noexcept {
            data = vsub_f32(data, vdup_n_f32(val));
            return *this;
        }

        template <>
        constexpr Vec2T<int32_t>& Vec2T<int32_t>::operator -=(int32_t val) noexcept {
            data = vsub_s32(data, vdup_n_s32(val));
            return *this;
        }

        //// Mul ////
        template <>
        constexpr Vec2T<float>& Vec2T<float>::operator *=(float val) noexcept {
            data = vmul_f32(data, vdup_n_f32(val));
            return *this;
        }

        template <>
        constexpr Vec2T<int32_t>& Vec2T<int32_t>::operator *=(int32_t val) noexcept {
            data = vmul_s32(data, vdup_n_s32(val));
            return *this;
        }

        //// Div ////
        #ifdef __arm64__
            template <>
            constexpr Vec2T<float>& Vec2T<float>::operator /=(float val) noexcept {
                data = vdiv_f32(data, vdup_n_f32(val));
                return *this;
            }
        #endif

        ////////////////////////////////////////////////////////////////////////////////////////////

        //// Add ////
        template <>
        constexpr Vec2T<float>& Vec2T<float>::operator +=(const Vec2T<float>& oth) noexcept {
            data = vadd_f32(data, oth.data);
            return *this;
        }

        template <>
        constexpr Vec2T<int32_t>& Vec2T<int32_t>::operator +=(const Vec2T<int32_t>& oth) noexcept {
            data = vadd_s32(data, oth.data);
            return *this;
        }

        //// Sub ////
        template <>
        constexpr Vec2T<float>& Vec2T<float>::operator -=(const Vec2T<float>& oth) noexcept {
            data = vsub_f32(data, oth.data);
            return *this;
        }

        template <>
        constexpr Vec2T<int32_t>& Vec2T<int32_t>::operator -=(const Vec2T<int32_t>& oth) noexcept {
            data = vsub_s32(data, oth.data);
            return *this;
        }

        //// Mum ////
        template <>
        constexpr Vec2T<float>& Vec2T<float>::operator *=(const Vec2T<float>& oth) noexcept {
            data = vmul_f32(data, oth.data);
            return *this;
        }

        template <>
        constexpr Vec2T<int32_t>& Vec2T<int32_t>::operator *=(const Vec2T<int32_t>& oth) noexcept {
            data = vmul_s32(data, oth.data);
            return *this;
        }

        //// Div ////
        #ifdef __arm64__
            template <>
            constexpr Vec2T<float>& Vec2T<float>::operator /=(const Vec2T<float>& oth) noexcept {
                data = vdiv_f32(data, oth.data);
                return *this;
            }
        #endif

        static constexpr float length(const Vec2T<float>& vec) noexcept {
            return static_cast<float>(sqrt((vec.x * vec.x) + (vec.y * vec.y)));
        }

        static constexpr float distance(const Vec2T<float>& vecA, const Vec2T<float>& vecB) noexcept {
            return length(vecB - vecA);
        }
    }

#endif

#endif