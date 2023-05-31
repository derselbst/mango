/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2023 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#pragma once

#include <mango/simd/simd.hpp>

namespace mango::simd
{

#define SIMD_COMPOSITE_FUNC1(R, A, FUNC) \
    static inline R FUNC(A a) \
    { \
        R result; \
        result.part[0] = FUNC(a.part[0]); \
        result.part[1] = FUNC(a.part[1]); \
        return result; \
    }

#define SIMD_COMPOSITE_FUNC2(R, AB, FUNC) \
    static inline R FUNC(AB a, AB b) \
    { \
        R result; \
        result.part[0] = FUNC(a.part[0], b.part[0]); \
        result.part[1] = FUNC(a.part[1], b.part[1]); \
        return result; \
    }

#define SIMD_COMPOSITE_FUNC3(R, ABC, FUNC) \
    static inline R FUNC(ABC a, ABC b, ABC c) \
    { \
        R result; \
        result.part[0] = FUNC(a.part[0], b.part[0], c.part[0]); \
        result.part[1] = FUNC(a.part[1], b.part[1], c.part[1]); \
        return result; \
    }

#define SIMD_COMPOSITE_ZEROMASK_FUNC2(R, AB, MASK, FUNC) \
    static inline R FUNC(AB a, AB b, MASK mask) \
    { \
        R result; \
        result.part[0] = FUNC(a.part[0], b.part[0], mask.part[0]); \
        result.part[1] = FUNC(a.part[1], b.part[1], mask.part[1]); \
        return result; \
    }

#define SIMD_COMPOSITE_MASK_FUNC2(R, AB, MASK, FUNC) \
    static inline R FUNC(AB a, AB b, MASK mask, AB value) \
    { \
        R result; \
        result.part[0] = FUNC(a.part[0], b.part[0], mask.part[0], value.part[0]); \
        result.part[1] = FUNC(a.part[1], b.part[1], mask.part[1], value.part[1]); \
        return result; \
    }

    // -----------------------------------------------------------------
    // f32x16
    // -----------------------------------------------------------------

    static inline f32x16 f32x16_zero()
    {
        f32x16 result;
        result.part[0] = f32x8_zero();
        result.part[1] = f32x8_zero();
        return result;
    }

    static inline f32x16 f32x16_set(f32 s)
    {
        f32x16 result;
        result.part[0] = f32x8_set(s);
        result.part[1] = f32x8_set(s);
        return result;
    }

    static inline f32x16 f32x16_set(
        f32 s0, f32 s1, f32 s2, f32 s3, f32 s4, f32 s5, f32 s6, f32 s7,
        f32 s8, f32 s9, f32 s10, f32 s11, f32 s12, f32 s13, f32 s14, f32 s15)
    {
        f32x16 result;
        result.part[0] = f32x8_set(s0, s1, s2, s3, s4, s5, s6, s7);
        result.part[1] = f32x8_set(s8, s9, s10, s11, s12, s13, s14, s15);
        return result;
    }

    static inline f32x16 f32x16_uload(const void* source)
    {
        f32x16 result;
        result.part[0] = f32x8_uload(reinterpret_cast<const f32*>(source) + 0);
        result.part[1] = f32x8_uload(reinterpret_cast<const f32*>(source) + 8);
        return result;
    }

    static inline void f32x16_ustore(void* dest, f32x16 a)
    {
        f32x8_ustore(reinterpret_cast<f32*>(dest) + 0, a.part[0]);
        f32x8_ustore(reinterpret_cast<f32*>(dest) + 8, a.part[1]);
    }

    SIMD_COMPOSITE_FUNC2(f32x16, f32x16, unpackhi)
    SIMD_COMPOSITE_FUNC2(f32x16, f32x16, unpacklo)
    SIMD_COMPOSITE_FUNC2(f32x16, f32x16, bitwise_nand)
    SIMD_COMPOSITE_FUNC2(f32x16, f32x16, bitwise_and)
    SIMD_COMPOSITE_FUNC2(f32x16, f32x16, bitwise_or)
    SIMD_COMPOSITE_FUNC2(f32x16, f32x16, bitwise_xor)
    SIMD_COMPOSITE_FUNC1(f32x16, f32x16, bitwise_not)
    SIMD_COMPOSITE_FUNC2(f32x16, f32x16, min)
    SIMD_COMPOSITE_FUNC2(f32x16, f32x16, max)
    SIMD_COMPOSITE_FUNC1(f32x16, f32x16, abs)
    SIMD_COMPOSITE_FUNC1(f32x16, f32x16, neg)
    SIMD_COMPOSITE_FUNC1(f32x16, f32x16, sign)
    SIMD_COMPOSITE_FUNC2(f32x16, f32x16, add)
    SIMD_COMPOSITE_FUNC2(f32x16, f32x16, sub)
    SIMD_COMPOSITE_FUNC2(f32x16, f32x16, mul)
    SIMD_COMPOSITE_FUNC2(f32x16, f32x16, div)

    SIMD_COMPOSITE_ZEROMASK_FUNC2(f32x16, f32x16, mask32x16, min)
    SIMD_COMPOSITE_ZEROMASK_FUNC2(f32x16, f32x16, mask32x16, max)
    SIMD_COMPOSITE_ZEROMASK_FUNC2(f32x16, f32x16, mask32x16, add)
    SIMD_COMPOSITE_ZEROMASK_FUNC2(f32x16, f32x16, mask32x16, sub)
    SIMD_COMPOSITE_ZEROMASK_FUNC2(f32x16, f32x16, mask32x16, mul)
    SIMD_COMPOSITE_ZEROMASK_FUNC2(f32x16, f32x16, mask32x16, div)
    SIMD_COMPOSITE_MASK_FUNC2(f32x16, f32x16, mask32x16, min)
    SIMD_COMPOSITE_MASK_FUNC2(f32x16, f32x16, mask32x16, max)
    SIMD_COMPOSITE_MASK_FUNC2(f32x16, f32x16, mask32x16, add)
    SIMD_COMPOSITE_MASK_FUNC2(f32x16, f32x16, mask32x16, sub)
    SIMD_COMPOSITE_MASK_FUNC2(f32x16, f32x16, mask32x16, mul)
    SIMD_COMPOSITE_MASK_FUNC2(f32x16, f32x16, mask32x16, div)

    static inline f32x16 div(f32x16 a, f32 b)
    {
        f32x16 result;
        result.part[0] = div(a.part[0], b);
        result.part[1] = div(a.part[1], b);
        return result;
    }

    SIMD_COMPOSITE_FUNC3(f32x16, f32x16, madd)
    SIMD_COMPOSITE_FUNC3(f32x16, f32x16, msub)
    SIMD_COMPOSITE_FUNC3(f32x16, f32x16, nmadd)
    SIMD_COMPOSITE_FUNC3(f32x16, f32x16, nmsub)
    SIMD_COMPOSITE_FUNC3(f32x16, f32x16, lerp)
    SIMD_COMPOSITE_FUNC1(f32x16, f32x16, rcp)
    SIMD_COMPOSITE_FUNC1(f32x16, f32x16, rsqrt)
    SIMD_COMPOSITE_FUNC1(f32x16, f32x16, sqrt)

    // compare

    SIMD_COMPOSITE_FUNC2(mask32x16, f32x16, compare_neq)
    SIMD_COMPOSITE_FUNC2(mask32x16, f32x16, compare_eq)
    SIMD_COMPOSITE_FUNC2(mask32x16, f32x16, compare_lt)
    SIMD_COMPOSITE_FUNC2(mask32x16, f32x16, compare_le)
    SIMD_COMPOSITE_FUNC2(mask32x16, f32x16, compare_gt)
    SIMD_COMPOSITE_FUNC2(mask32x16, f32x16, compare_ge)

    static inline f32x16 select(mask32x16 mask, f32x16 a, f32x16 b)
    {
        f32x16 result;
        result.part[0] = select(mask.part[0], a.part[0], b.part[0]);
        result.part[1] = select(mask.part[1], a.part[1], b.part[1]);
        return result;
    }

    // rounding

    SIMD_COMPOSITE_FUNC1(f32x16, f32x16, round)
    SIMD_COMPOSITE_FUNC1(f32x16, f32x16, trunc)
    SIMD_COMPOSITE_FUNC1(f32x16, f32x16, floor)
    SIMD_COMPOSITE_FUNC1(f32x16, f32x16, ceil)
    SIMD_COMPOSITE_FUNC1(f32x16, f32x16, fract)

#undef SIMD_COMPOSITE_FUNC1
#undef SIMD_COMPOSITE_FUNC2
#undef SIMD_COMPOSITE_FUNC3
#undef SIMD_COMPOSITE_ZEROMASK_FUNC2
#undef SIMD_COMPOSITE_MASK_FUNC2

} // namespace mango::simd
