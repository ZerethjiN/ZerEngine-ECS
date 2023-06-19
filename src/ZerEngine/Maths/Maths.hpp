#ifndef ZERENGINE_MATHS
#define ZERENGINE_MATHS

#include "Numerics.hpp"
#include "Trigonometric.hpp"
#include "Projection.hpp"

#include "Vec2.hpp"

#include "Vec3.hpp"
#include "Vec4.hpp"

#ifdef __SSE2__
    #include "Vec4SSE.hpp"
#endif

#ifdef __AVX2__
    #include "Vec4AVX.hpp"
#endif

#ifdef __ARM_NEON
    #include "Vec2NEON.hpp"
    #include "Vec4NEON.hpp"
#endif

#include "Mat3.hpp"
#include "Mat4.hpp"

#endif