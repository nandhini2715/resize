/*M///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                          License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2000-2008, Intel Corporation, all rights reserved.
// Copyright (C) 2009, Willow Garage Inc., all rights reserved.
// Copyright (C) 2013, OpenCV Foundation, all rights reserved.
// Copyright (C) 2015, Itseez Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

#ifndef OPENCV_HAL_INTRIN_HPP
#define OPENCV_HAL_INTRIN_HPP

#include <cmath>
#include <float.h>
#include <stdlib.h>
#include "opencv2/core/cvdef.h"

#if defined(__GNUC__) && __GNUC__ == 12
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuninitialized"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

#define OPENCV_HAL_ADD(a, b) ((a) + (b))
#define OPENCV_HAL_AND(a, b) ((a) & (b))
#define OPENCV_HAL_NOP(a) (a)
#define OPENCV_HAL_1ST(a, b) (a)

namespace {
inline unsigned int trailingZeros32(unsigned int value) {
#if defined(_MSC_VER)
#if (_MSC_VER < 1700) || defined(_M_ARM) || defined(_M_ARM64)
    unsigned long index = 0;
    _BitScanForward(&index, value);
    return (unsigned int)index;
#elif defined(__clang__)
    // clang-cl doesn't export _tzcnt_u32 for non BMI systems
    return value ? __builtin_ctz(value) : 32;
#else
    return _tzcnt_u32(value);
#endif
#elif defined(__GNUC__) || defined(__GNUG__)
    return __builtin_ctz(value);
#elif defined(__ICC) || defined(__INTEL_COMPILER)
    return _bit_scan_forward(value);
#elif defined(__clang__)
    return llvm.cttz.i32(value, true);
#else
    static const int MultiplyDeBruijnBitPosition[32] = {
        0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8,
        31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9 };
    return MultiplyDeBruijnBitPosition[((uint32_t)((value & -value) * 0x077CB531U)) >> 27];
#endif
}
}

// unlike HAL API, which is in cv::hal,
// we put intrinsics into cv namespace to make its
// access from within opencv code more accessible
namespace cv {

namespace hal {

enum StoreMode
{
    STORE_UNALIGNED = 0,
    STORE_ALIGNED = 1,
    STORE_ALIGNED_NOCACHE = 2
};

}

// TODO FIXIT: Don't use "God" traits. Split on separate cases.
template<typename _Tp> struct V_TypeTraits
{
};

#define CV_INTRIN_DEF_TYPE_TRAITS(type, int_type_, uint_type_, abs_type_, w_type_, q_type_, sum_type_) \
    template<> struct V_TypeTraits<type> \
    { \
        typedef type value_type; \
        typedef int_type_ int_type; \
        typedef abs_type_ abs_type; \
        typedef uint_type_ uint_type; \
        typedef w_type_ w_type; \
        typedef q_type_ q_type; \
        typedef sum_type_ sum_type; \
    \
        static inline int_type reinterpret_int(type x) \
        { \
            union { type l; int_type i; } v; \
            v.l = x; \
            return v.i; \
        } \
    \
        static inline type reinterpret_from_int(int_type x) \
        { \
            union { type l; int_type i; } v; \
            v.i = x; \
            return v.l; \
        } \
    }

#define CV_INTRIN_DEF_TYPE_TRAITS_NO_Q_TYPE(type, int_type_, uint_type_, abs_type_, w_type_, sum_type_) \
    template<> struct V_TypeTraits<type> \
    { \
        typedef type value_type; \
        typedef int_type_ int_type; \
        typedef abs_type_ abs_type; \
        typedef uint_type_ uint_type; \
        typedef w_type_ w_type; \
        typedef sum_type_ sum_type; \
    \
        static inline int_type reinterpret_int(type x) \
        { \
            union { type l; int_type i; } v; \
            v.l = x; \
            return v.i; \
        } \
    \
        static inline type reinterpret_from_int(int_type x) \
        { \
            union { type l; int_type i; } v; \
            v.i = x; \
            return v.l; \
        } \
    }

CV_INTRIN_DEF_TYPE_TRAITS(uchar, schar, uchar, uchar, ushort, unsigned, unsigned);
CV_INTRIN_DEF_TYPE_TRAITS(schar, schar, uchar, uchar, short, int, int);
CV_INTRIN_DEF_TYPE_TRAITS(ushort, short, ushort, ushort, unsigned, uint64, unsigned);
CV_INTRIN_DEF_TYPE_TRAITS(short, short, ushort, ushort, int, int64, int);
CV_INTRIN_DEF_TYPE_TRAITS_NO_Q_TYPE(unsigned, int, unsigned, unsigned, uint64, unsigned);
CV_INTRIN_DEF_TYPE_TRAITS_NO_Q_TYPE(int, int, unsigned, unsigned, int64, int);
CV_INTRIN_DEF_TYPE_TRAITS_NO_Q_TYPE(float, int, unsigned, float, double, float);
CV_INTRIN_DEF_TYPE_TRAITS_NO_Q_TYPE(uint64, int64, uint64, uint64, void, uint64);
CV_INTRIN_DEF_TYPE_TRAITS_NO_Q_TYPE(int64, int64, uint64, uint64, void, int64);
CV_INTRIN_DEF_TYPE_TRAITS_NO_Q_TYPE(double, int64, uint64, double, void, double);

#ifndef CV_DOXYGEN

#ifndef CV_CPU_OPTIMIZATION_HAL_NAMESPACE
#ifdef CV_FORCE_SIMD128_CPP
    #define CV_CPU_OPTIMIZATION_HAL_NAMESPACE hal_EMULATOR_CPP
    #define CV_CPU_OPTIMIZATION_HAL_NAMESPACE_BEGIN namespace hal_EMULATOR_CPP {
    #define CV_CPU_OPTIMIZATION_HAL_NAMESPACE_END }
#elif defined(CV_CPU_DISPATCH_MODE)
    #define CV_CPU_OPTIMIZATION_HAL_NAMESPACE __CV_CAT(hal_, CV_CPU_DISPATCH_MODE)
    #define CV_CPU_OPTIMIZATION_HAL_NAMESPACE_BEGIN namespace __CV_CAT(hal_, CV_CPU_DISPATCH_MODE) {
    #define CV_CPU_OPTIMIZATION_HAL_NAMESPACE_END }
#else
    #define CV_CPU_OPTIMIZATION_HAL_NAMESPACE hal_baseline
    #define CV_CPU_OPTIMIZATION_HAL_NAMESPACE_BEGIN namespace hal_baseline {
    #define CV_CPU_OPTIMIZATION_HAL_NAMESPACE_END }
#endif
#endif // CV_CPU_OPTIMIZATION_HAL_NAMESPACE

CV_CPU_OPTIMIZATION_HAL_NAMESPACE_BEGIN
CV_CPU_OPTIMIZATION_HAL_NAMESPACE_END
using namespace CV_CPU_OPTIMIZATION_HAL_NAMESPACE;
#endif
}

#ifdef CV_DOXYGEN
#   undef CV_AVX2
#   undef CV_SSE2
#   undef CV_NEON
#   undef CV_VSX
#   undef CV_FP16
#   undef CV_MSA
#   undef CV_RVV
#endif

#if (CV_SSE2 || CV_NEON || CV_VSX || CV_MSA || CV_WASM_SIMD || CV_RVV071 || CV_LSX) && !defined(CV_FORCE_SIMD128_CPP)
#define CV__SIMD_FORWARD 128
#include "opencv2/core/hal/intrin_forward.hpp"
#endif

#if CV_SSE2 && !defined(CV_FORCE_SIMD128_CPP)

#include "opencv2/core/hal/intrin_sse_em.hpp"
#include "opencv2/core/hal/intrin_sse.hpp"

#elif CV_NEON && !defined(CV_FORCE_SIMD128_CPP)

#include "opencv2/core/hal/intrin_neon.hpp"

#elif CV_RVV071 && !defined(CV_FORCE_SIMD128_CPP)
#define CV_SIMD128_CPP 0
#include "opencv2/core/hal/intrin_rvv071.hpp"

#elif CV_VSX && !defined(CV_FORCE_SIMD128_CPP)

#include "opencv2/core/hal/intrin_vsx.hpp"

#elif CV_MSA && !defined(CV_FORCE_SIMD128_CPP)

#include "opencv2/core/hal/intrin_msa.hpp"

#elif CV_WASM_SIMD && !defined(CV_FORCE_SIMD128_CPP)
#include "opencv2/core/hal/intrin_wasm.hpp"

#elif CV_RVV && !defined(CV_FORCE_SIMD128_CPP)
#if defined(CV_RVV_SCALABLE)
#include "opencv2/core/hal/intrin_rvv_scalable.hpp"
#else
#include "opencv2/core/hal/intrin_rvv.hpp"
#endif

#elif CV_LSX && !defined(CV_FORCE_SIMD128_CPP)

#include "opencv2/core/hal/intrin_lsx.hpp"

#else

#include "opencv2/core/hal/intrin_cpp.hpp"

#endif

// AVX2 can be used together with SSE2, so
// we define those two sets of intrinsics at once.
// Most of the intrinsics do not conflict (the proper overloaded variant is
// resolved by the argument types, e.g. v_float32x4 ~ SSE2, v_float32x8 ~ AVX2),
// but some of AVX2 intrinsics get v256_ prefix instead of v_, e.g. v256_load() vs v_load().
// Correspondingly, the wide intrinsics (which are mapped to the "widest"
// available instruction set) will get vx_ prefix
// (and will be mapped to v256_ counterparts) (e.g. vx_load() => v256_load())
#if CV_AVX2

#define CV__SIMD_FORWARD 256
#include "opencv2/core/hal/intrin_forward.hpp"
#include "opencv2/core/hal/intrin_avx.hpp"

#endif

// AVX512 can be used together with SSE2 and AVX2, so
// we define those sets of intrinsics at once.
// For some of AVX512 intrinsics get v512_ prefix instead of v_, e.g. v512_load() vs v_load().
// Wide intrinsics will be mapped to v512_ counterparts in this case(e.g. vx_load() => v512_load())
#if CV_AVX512_SKX

#define CV__SIMD_FORWARD 512
#include "opencv2/core/hal/intrin_forward.hpp"
#include "opencv2/core/hal/intrin_avx512.hpp"

#endif

#if CV_LASX

#define CV__SIMD_FORWARD 256
#include "opencv2/core/hal/intrin_forward.hpp"
#include "opencv2/core/hal/intrin_lasx.hpp"

#endif

//! @cond IGNORED

namespace cv {

#ifndef CV_DOXYGEN
CV_CPU_OPTIMIZATION_HAL_NAMESPACE_BEGIN
#endif

#ifndef CV_SIMD128
#define CV_SIMD128 0
#endif

#ifndef CV_SIMD128_CPP
#define CV_SIMD128_CPP 0
#endif

#ifndef CV_SIMD128_64F
#define CV_SIMD128_64F 0
#endif

#ifndef CV_SIMD256
#define CV_SIMD256 0
#endif

#ifndef CV_SIMD256_64F
#define CV_SIMD256_64F 0
#endif

#ifndef CV_SIMD512
#define CV_SIMD512 0
#endif

#ifndef CV_SIMD512_64F
#define CV_SIMD512_64F 0
#endif

#ifndef CV_SIMD128_FP16
#define CV_SIMD128_FP16 0
#endif

#ifndef CV_SIMD256_FP16
#define CV_SIMD256_FP16 0
#endif

#ifndef CV_SIMD512_FP16
#define CV_SIMD512_FP16 0
#endif

#ifndef CV_SIMD_SCALABLE
#define CV_SIMD_SCALABLE 0
#endif

#ifndef CV_SIMD_SCALABLE_64F
#define CV_SIMD_SCALABLE_64F 0
#endif

//==================================================================================================

template<typename _Tp> struct V_RegTraits
{
};

#define CV_DEF_REG_TRAITS(prefix, _reg, lane_type, suffix, _u_reg, _w_reg, _q_reg, _int_reg, _round_reg) \
    template<> struct V_RegTraits<_reg> \
    { \
        typedef _reg reg; \
        typedef _u_reg u_reg; \
        typedef _w_reg w_reg; \
        typedef _q_reg q_reg; \
        typedef _int_reg int_reg; \
        typedef _round_reg round_reg; \
    }

#if CV_SIMD128 || CV_SIMD128_CPP
    CV_DEF_REG_TRAITS(v, v_uint8x16, uchar, u8, v_uint8x16, v_uint16x8, v_uint32x4, v_int8x16, void);
    CV_DEF_REG_TRAITS(v, v_int8x16, schar, s8, v_uint8x16, v_int16x8, v_int32x4, v_int8x16, void);
    CV_DEF_REG_TRAITS(v, v_uint16x8, ushort, u16, v_uint16x8, v_uint32x4, v_uint64x2, v_int16x8, void);
    CV_DEF_REG_TRAITS(v, v_int16x8, short, s16, v_uint16x8, v_int32x4, v_int64x2, v_int16x8, void);
    CV_DEF_REG_TRAITS(v, v_uint32x4, unsigned, u32, v_uint32x4, v_uint64x2, void, v_int32x4, void);
    CV_DEF_REG_TRAITS(v, v_int32x4, int, s32, v_uint32x4, v_int64x2, void, v_int32x4, void);
#if CV_SIMD128_64F || CV_SIMD128_CPP
    CV_DEF_REG_TRAITS(v, v_float32x4, float, f32, v_float32x4, v_float64x2, void, v_int32x4, v_int32x4);
#else
    CV_DEF_REG_TRAITS(v, v_float32x4, float, f32, v_float32x4, void, void, v_int32x4, v_int32x4);
#endif
    CV_DEF_REG_TRAITS(v, v_uint64x2, uint64, u64, v_uint64x2, void, void, v_int64x2, void);
    CV_DEF_REG_TRAITS(v, v_int64x2, int64, s64, v_uint64x2, void, void, v_int64x2, void);
#if CV_SIMD128_64F
    CV_DEF_REG_TRAITS(v, v_float64x2, double, f64, v_float64x2, void, void, v_int64x2, v_int32x4);
#endif
#endif

#if CV_SIMD256
    CV_DEF_REG_TRAITS(v256, v_uint8x32, uchar, u8, v_uint8x32, v_uint16x16, v_uint32x8, v_int8x32, void);
    CV_DEF_REG_TRAITS(v256, v_int8x32, schar, s8, v_uint8x32, v_int16x16, v_int32x8, v_int8x32, void);
    CV_DEF_REG_TRAITS(v256, v_uint16x16, ushort, u16, v_uint16x16, v_uint32x8, v_uint64x4, v_int16x16, void);
    CV_DEF_REG_TRAITS(v256, v_int16x16, short, s16, v_uint16x16, v_int32x8, v_int64x4, v_int16x16, void);
    CV_DEF_REG_TRAITS(v256, v_uint32x8, unsigned, u32, v_uint32x8, v_uint64x4, void, v_int32x8, void);
    CV_DEF_REG_TRAITS(v256, v_int32x8, int, s32, v_uint32x8, v_int64x4, void, v_int32x8, void);
    CV_DEF_REG_TRAITS(v256, v_float32x8, float, f32, v_float32x8, v_float64x4, void, v_int32x8, v_int32x8);
    CV_DEF_REG_TRAITS(v256, v_uint64x4, uint64, u64, v_uint64x4, void, void, v_int64x4, void);
    CV_DEF_REG_TRAITS(v256, v_int64x4, int64, s64, v_uint64x4, void, void, v_int64x4, void);
    CV_DEF_REG_TRAITS(v256, v_float64x4, double, f64, v_float64x4, void, void, v_int64x4, v_int32x8);
#endif

#if CV_SIMD512
    CV_DEF_REG_TRAITS(v512, v_uint8x64, uchar, u8, v_uint8x64, v_uint16x32, v_uint32x16, v_int8x64, void);
    CV_DEF_REG_TRAITS(v512, v_int8x64, schar, s8, v_uint8x64, v_int16x32, v_int32x16, v_int8x64, void);
    CV_DEF_REG_TRAITS(v512, v_uint16x32, ushort, u16, v_uint16x32, v_uint32x16, v_uint64x8, v_int16x32, void);
    CV_DEF_REG_TRAITS(v512, v_int16x32, short, s16, v_uint16x32, v_int32x16, v_int64x8, v_int16x32, void);
    CV_DEF_REG_TRAITS(v512, v_uint32x16, unsigned, u32, v_uint32x16, v_uint64x8, void, v_int32x16, void);
    CV_DEF_REG_TRAITS(v512, v_int32x16, int, s32, v_uint32x16, v_int64x8, void, v_int32x16, void);
    CV_DEF_REG_TRAITS(v512, v_float32x16, float, f32, v_float32x16, v_float64x8, void, v_int32x16, v_int32x16);
    CV_DEF_REG_TRAITS(v512, v_uint64x8, uint64, u64, v_uint64x8, void, void, v_int64x8, void);
    CV_DEF_REG_TRAITS(v512, v_int64x8, int64, s64, v_uint64x8, void, void, v_int64x8, void);
    CV_DEF_REG_TRAITS(v512, v_float64x8, double, f64, v_float64x8, void, void, v_int64x8, v_int32x16);
#endif
#if CV_SIMD_SCALABLE
    CV_DEF_REG_TRAITS(v, v_uint8, uchar, u8, v_uint8, v_uint16, v_uint32, v_int8, void);
    CV_DEF_REG_TRAITS(v, v_int8, schar, s8, v_uint8, v_int16, v_int32, v_int8, void);
    CV_DEF_REG_TRAITS(v, v_uint16, ushort, u16, v_uint16, v_uint32, v_uint64, v_int16, void);
    CV_DEF_REG_TRAITS(v, v_int16, short, s16, v_uint16, v_int32, v_int64, v_int16, void);
    CV_DEF_REG_TRAITS(v, v_uint32, unsigned, u32, v_uint32, v_uint64, void, v_int32, void);
    CV_DEF_REG_TRAITS(v, v_int32, int, s32, v_uint32, v_int64, void, v_int32, void);
    CV_DEF_REG_TRAITS(v, v_float32, float, f32, v_float32, v_float64, void, v_int32, v_int32);
    CV_DEF_REG_TRAITS(v, v_uint64, uint64, u64, v_uint64, void, void, v_int64, void);
    CV_DEF_REG_TRAITS(v, v_int64, int64, s64, v_uint64, void, void, v_int64, void);
    CV_DEF_REG_TRAITS(v, v_float64, double, f64, v_float64, void, void, v_int64, v_int32);
#endif
//! @endcond

#if CV_SIMD512 && (!defined(CV__SIMD_FORCE_WIDTH) || CV__SIMD_FORCE_WIDTH == 512)
#define CV__SIMD_NAMESPACE simd512
namespace CV__SIMD_NAMESPACE {
    #define CV_SIMD 1
    #define CV_SIMD_64F CV_SIMD512_64F
    #define CV_SIMD_FP16 CV_SIMD512_FP16
    #define CV_SIMD_WIDTH 64
//! @addtogroup core_hal_intrin
//! @{
    //! @brief Maximum available vector register capacity 8-bit unsigned integer values
    typedef v_uint8x64    v_uint8;
    //! @brief Maximum available vector register capacity 8-bit signed integer values
    typedef v_int8x64     v_int8;
    //! @brief Maximum available vector register capacity 16-bit unsigned integer values
    typedef v_uint16x32   v_uint16;
    //! @brief Maximum available vector register capacity 16-bit signed integer values
    typedef v_int16x32    v_int16;
    //! @brief Maximum available vector register capacity 32-bit unsigned integer values
    typedef v_uint32x16   v_uint32;
    //! @brief Maximum available vector register capacity 32-bit signed integer values
    typedef v_int32x16    v_int32;
    //! @brief Maximum available vector register capacity 64-bit unsigned integer values
    typedef v_uint64x8    v_uint64;
    //! @brief Maximum available vector register capacity 64-bit signed integer values
    typedef v_int64x8     v_int64;
    //! @brief Maximum available vector register capacity 32-bit floating point values (single precision)
    typedef v_float32x16  v_float32;
    #if CV_SIMD512_64F
    //! @brief Maximum available vector register capacity 64-bit floating point values (double precision)
    typedef v_float64x8   v_float64;
    #endif
//! @}

    #define VXPREFIX(func) v512##func
} // namespace
using namespace CV__SIMD_NAMESPACE;
#elif CV_SIMD256 && (!defined(CV__SIMD_FORCE_WIDTH) || CV__SIMD_FORCE_WIDTH == 256)
#define CV__SIMD_NAMESPACE simd256
namespace CV__SIMD_NAMESPACE {
    #define CV_SIMD 1
    #define CV_SIMD_64F CV_SIMD256_64F
    #define CV_SIMD_FP16 CV_SIMD256_FP16
    #define CV_SIMD_WIDTH 32
//! @addtogroup core_hal_intrin
//! @{
    //! @brief Maximum available vector register capacity 8-bit unsigned integer values
    typedef v_uint8x32   v_uint8;
    //! @brief Maximum available vector register capacity 8-bit signed integer values
    typedef v_int8x32    v_int8;
    //! @brief Maximum available vector register capacity 16-bit unsigned integer values
    typedef v_uint16x16  v_uint16;
    //! @brief Maximum available vector register capacity 16-bit signed integer values
    typedef v_int16x16   v_int16;
    //! @brief Maximum available vector register capacity 32-bit unsigned integer values
    typedef v_uint32x8   v_uint32;
    //! @brief Maximum available vector register capacity 32-bit signed integer values
    typedef v_int32x8    v_int32;
    //! @brief Maximum available vector register capacity 64-bit unsigned integer values
    typedef v_uint64x4   v_uint64;
    //! @brief Maximum available vector register capacity 64-bit signed integer values
    typedef v_int64x4    v_int64;
    //! @brief Maximum available vector register capacity 32-bit floating point values (single precision)
    typedef v_float32x8  v_float32;
    #if CV_SIMD256_64F
    //! @brief Maximum available vector register capacity 64-bit floating point values (double precision)
    typedef v_float64x4  v_float64;
    #endif
//! @}

    #define VXPREFIX(func) v256##func
} // namespace
using namespace CV__SIMD_NAMESPACE;
#elif (CV_SIMD128 || CV_SIMD128_CPP) && (!defined(CV__SIMD_FORCE_WIDTH) || CV__SIMD_FORCE_WIDTH == 128)
#if defined CV_SIMD128_CPP
#define CV__SIMD_NAMESPACE simd128_cpp
#else
#define CV__SIMD_NAMESPACE simd128
#endif
namespace CV__SIMD_NAMESPACE {
    #define CV_SIMD CV_SIMD128
    #define CV_SIMD_64F CV_SIMD128_64F
    #define CV_SIMD_WIDTH 16
//! @addtogroup core_hal_intrin
//! @{
    //! @brief Maximum available vector register capacity 8-bit unsigned integer values
    typedef v_uint8x16  v_uint8;
    //! @brief Maximum available vector register capacity 8-bit signed integer values
    typedef v_int8x16   v_int8;
    //! @brief Maximum available vector register capacity 16-bit unsigned integer values
    typedef v_uint16x8  v_uint16;
    //! @brief Maximum available vector register capacity 16-bit signed integer values
    typedef v_int16x8   v_int16;
    //! @brief Maximum available vector register capacity 32-bit unsigned integer values
    typedef v_uint32x4  v_uint32;
    //! @brief Maximum available vector register capacity 32-bit signed integer values
    typedef v_int32x4   v_int32;
    //! @brief Maximum available vector register capacity 64-bit unsigned integer values
    typedef v_uint64x2  v_uint64;
    //! @brief Maximum available vector register capacity 64-bit signed integer values
    typedef v_int64x2   v_int64;
    //! @brief Maximum available vector register capacity 32-bit floating point values (single precision)
    typedef v_float32x4 v_float32;
    #if CV_SIMD128_64F
    //! @brief Maximum available vector register capacity 64-bit floating point values (double precision)
    typedef v_float64x2 v_float64;
    #endif
//! @}

    #define VXPREFIX(func) v##func
} // namespace
using namespace CV__SIMD_NAMESPACE;

#elif CV_SIMD_SCALABLE
#define CV__SIMD_NAMESPACE simd
namespace CV__SIMD_NAMESPACE {
    #define CV_SIMD 0
    #define CV_SIMD_WIDTH 128  /* 1024/8 */

    #define VXPREFIX(func) v##func
} // namespace
using namespace CV__SIMD_NAMESPACE;

#endif

//! @cond IGNORED
#ifndef CV_SIMD_64F
#define CV_SIMD_64F 0
#endif

namespace CV__SIMD_NAMESPACE {
//! @addtogroup core_hal_intrin
//! @{
    //! @name Wide init with value
    //! @{
    //! @brief Create maximum available capacity vector with elements set to a specific value
    inline v_uint8 vx_setall_u8(uchar v) { return VXPREFIX(_setall_u8)(v); }
    inline v_int8 vx_setall_s8(schar v) { return VXPREFIX(_setall_s8)(v); }
    inline v_uint16 vx_setall_u16(ushort v) { return VXPREFIX(_setall_u16)(v); }
    inline v_int16 vx_setall_s16(short v) { return VXPREFIX(_setall_s16)(v); }
    inline v_int32 vx_setall_s32(int v) { return VXPREFIX(_setall_s32)(v); }
    inline v_uint32 vx_setall_u32(unsigned v) { return VXPREFIX(_setall_u32)(v); }
    inline v_float32 vx_setall_f32(float v) { return VXPREFIX(_setall_f32)(v); }
    inline v_int64 vx_setall_s64(int64 v) { return VXPREFIX(_setall_s64)(v); }
    inline v_uint64 vx_setall_u64(uint64 v) { return VXPREFIX(_setall_u64)(v); }
#if CV_SIMD_64F || CV_SIMD_SCALABLE_64F
    inline v_float64 vx_setall_f64(double v) { return VXPREFIX(_setall_f64)(v); }
#endif
    //! @}

    //! @name Wide init with zero
    //! @{
    //! @brief Create maximum available capacity vector with elements set to zero
    inline v_uint8 vx_setzero_u8() { return VXPREFIX(_setzero_u8)(); }
    inline v_int8 vx_setzero_s8() { return VXPREFIX(_setzero_s8)(); }
    inline v_uint16 vx_setzero_u16() { return VXPREFIX(_setzero_u16)(); }
    inline v_int16 vx_setzero_s16() { return VXPREFIX(_setzero_s16)(); }
    inline v_int32 vx_setzero_s32() { return VXPREFIX(_setzero_s32)(); }
    inline v_uint32 vx_setzero_u32() { return VXPREFIX(_setzero_u32)(); }
    inline v_float32 vx_setzero_f32() { return VXPREFIX(_setzero_f32)(); }
    inline v_int64 vx_setzero_s64() { return VXPREFIX(_setzero_s64)(); }
    inline v_uint64 vx_setzero_u64() { return VXPREFIX(_setzero_u64)(); }
#if CV_SIMD_64F || CV_SIMD_SCALABLE_64F
    inline v_float64 vx_setzero_f64() { return VXPREFIX(_setzero_f64)(); }
#endif
    //! @}

    //! @name Wide load from memory
    //! @{
    //! @brief Load maximum available capacity register contents from memory
    inline v_uint8 vx_load(const uchar * ptr) { return VXPREFIX(_load)(ptr); }
    inline v_int8 vx_load(const schar * ptr) { return VXPREFIX(_load)(ptr); }
    inline v_uint16 vx_load(const ushort * ptr) { return VXPREFIX(_load)(ptr); }
    inline v_int16 vx_load(const short * ptr) { return VXPREFIX(_load)(ptr); }
    inline v_int32 vx_load(const int * ptr) { return VXPREFIX(_load)(ptr); }
    inline v_uint32 vx_load(const unsigned * ptr) { return VXPREFIX(_load)(ptr); }
    inline v_float32 vx_load(const float * ptr) { return VXPREFIX(_load)(ptr); }
    inline v_int64 vx_load(const int64 * ptr) { return VXPREFIX(_load)(ptr); }
    inline v_uint64 vx_load(const uint64 * ptr) { return VXPREFIX(_load)(ptr); }
#if CV_SIMD_64F || CV_SIMD_SCALABLE_64F
    inline v_float64 vx_load(const double * ptr) { return VXPREFIX(_load)(ptr); }
#endif
    //! @}

    //! @name Wide load from memory(aligned)
    //! @{
    //! @brief Load maximum available capacity register contents from memory(aligned)
    inline v_uint8 vx_load_aligned(const uchar * ptr) { return VXPREFIX(_load_aligned)(ptr); }
    inline v_int8 vx_load_aligned(const schar * ptr) { return VXPREFIX(_load_aligned)(ptr); }
    inline v_uint16 vx_load_aligned(const ushort * ptr) { return VXPREFIX(_load_aligned)(ptr); }
    inline v_int16 vx_load_aligned(const short * ptr) { return VXPREFIX(_load_aligned)(ptr); }
    inline v_int32 vx_load_aligned(const int * ptr) { return VXPREFIX(_load_aligned)(ptr); }
    inline v_uint32 vx_load_aligned(const unsigned * ptr) { return VXPREFIX(_load_aligned)(ptr); }
    inline v_float32 vx_load_aligned(const float * ptr) { return VXPREFIX(_load_aligned)(ptr); }
    inline v_int64 vx_load_aligned(const int64 * ptr) { return VXPREFIX(_load_aligned)(ptr); }
    inline v_uint64 vx_load_aligned(const uint64 * ptr) { return VXPREFIX(_load_aligned)(ptr); }
#if CV_SIMD_64F || CV_SIMD_SCALABLE_64F
    inline v_float64 vx_load_aligned(const double * ptr) { return VXPREFIX(_load_aligned)(ptr); }
#endif
    //! @}

    //! @name Wide load lower half from memory
    //! @{
    //! @brief Load lower half of maximum available capacity register from memory
    inline v_uint8 vx_load_low(const uchar * ptr) { return VXPREFIX(_load_low)(ptr); }
    inline v_int8 vx_load_low(const schar * ptr) { return VXPREFIX(_load_low)(ptr); }
    inline v_uint16 vx_load_low(const ushort * ptr) { return VXPREFIX(_load_low)(ptr); }
    inline v_int16 vx_load_low(const short * ptr) { return VXPREFIX(_load_low)(ptr); }
    inline v_int32 vx_load_low(const int * ptr) { return VXPREFIX(_load_low)(ptr); }
    inline v_uint32 vx_load_low(const unsigned * ptr) { return VXPREFIX(_load_low)(ptr); }
    inline v_float32 vx_load_low(const float * ptr) { return VXPREFIX(_load_low)(ptr); }
    inline v_int64 vx_load_low(const int64 * ptr) { return VXPREFIX(_load_low)(ptr); }
    inline v_uint64 vx_load_low(const uint64 * ptr) { return VXPREFIX(_load_low)(ptr); }
#if CV_SIMD_64F || CV_SIMD_SCALABLE_64F
    inline v_float64 vx_load_low(const double * ptr) { return VXPREFIX(_load_low)(ptr); }
#endif
    //! @}

    //! @name Wide load halfs from memory
    //! @{
    //! @brief Load maximum available capacity register contents from two memory blocks
    inline v_uint8 vx_load_halves(const uchar * ptr0, const uchar * ptr1) { return VXPREFIX(_load_halves)(ptr0, ptr1); }
    inline v_int8 vx_load_halves(const schar * ptr0, const schar * ptr1) { return VXPREFIX(_load_halves)(ptr0, ptr1); }
    inline v_uint16 vx_load_halves(const ushort * ptr0, const ushort * ptr1) { return VXPREFIX(_load_halves)(ptr0, ptr1); }
    inline v_int16 vx_load_halves(const short * ptr0, const short * ptr1) { return VXPREFIX(_load_halves)(ptr0, ptr1); }
    inline v_int32 vx_load_halves(const int * ptr0, const int * ptr1) { return VXPREFIX(_load_halves)(ptr0, ptr1); }
    inline v_uint32 vx_load_halves(const unsigned * ptr0, const unsigned * ptr1) { return VXPREFIX(_load_halves)(ptr0, ptr1); }
    inline v_float32 vx_load_halves(const float * ptr0, const float * ptr1) { return VXPREFIX(_load_halves)(ptr0, ptr1); }
    inline v_int64 vx_load_halves(const int64 * ptr0, const int64 * ptr1) { return VXPREFIX(_load_halves)(ptr0, ptr1); }
    inline v_uint64 vx_load_halves(const uint64 * ptr0, const uint64 * ptr1) { return VXPREFIX(_load_halves)(ptr0, ptr1); }
#if CV_SIMD_64F || CV_SIMD_SCALABLE_64F
    inline v_float64 vx_load_halves(const double * ptr0, const double * ptr1) { return VXPREFIX(_load_halves)(ptr0, ptr1); }
#endif
    //! @}

    //! @name Wide LUT of elements
    //! @{
    //! @brief Load maximum available capacity register contents with array elements by provided indexes
    inline v_uint8 vx_lut(const uchar * ptr, const int* idx) { return VXPREFIX(_lut)(ptr, idx); }
    inline v_int8 vx_lut(const schar * ptr, const int* idx) { return VXPREFIX(_lut)(ptr, idx); }
    inline v_uint16 vx_lut(const ushort * ptr, const int* idx) { return VXPREFIX(_lut)(ptr, idx); }
    inline v_int16 vx_lut(const short* ptr, const int* idx) { return VXPREFIX(_lut)(ptr, idx); }
    inline v_int32 vx_lut(const int* ptr, const int* idx) { return VXPREFIX(_lut)(ptr, idx); }
    inline v_uint32 vx_lut(const unsigned* ptr, const int* idx) { return VXPREFIX(_lut)(ptr, idx); }
    inline v_float32 vx_lut(const float* ptr, const int* idx) { return VXPREFIX(_lut)(ptr, idx); }
    inline v_int64 vx_lut(const int64 * ptr, const int* idx) { return VXPREFIX(_lut)(ptr, idx); }
    inline v_uint64 vx_lut(const uint64 * ptr, const int* idx) { return VXPREFIX(_lut)(ptr, idx); }
#if CV_SIMD_64F || CV_SIMD_SCALABLE_64F
    inline v_float64 vx_lut(const double* ptr, const int* idx) { return VXPREFIX(_lut)(ptr, idx); }
#endif
    //! @}

    //! @name Wide LUT of element pairs
    //! @{
    //! @brief Load maximum available capacity register contents with array element pairs by provided indexes
    inline v_uint8 vx_lut_pairs(const uchar * ptr, const int* idx) { return VXPREFIX(_lut_pairs)(ptr, idx); }
    inline v_int8 vx_lut_pairs(const schar * ptr, const int* idx) { return VXPREFIX(_lut_pairs)(ptr, idx); }
    inline v_uint16 vx_lut_pairs(const ushort * ptr, const int* idx) { return VXPREFIX(_lut_pairs)(ptr, idx); }
    inline v_int16 vx_lut_pairs(const short* ptr, const int* idx) { return VXPREFIX(_lut_pairs)(ptr, idx); }
    inline v_int32 vx_lut_pairs(const int* ptr, const int* idx) { return VXPREFIX(_lut_pairs)(ptr, idx); }
    inline v_uint32 vx_lut_pairs(const unsigned* ptr, const int* idx) { return VXPREFIX(_lut_pairs)(ptr, idx); }
    inline v_float32 vx_lut_pairs(const float* ptr, const int* idx) { return VXPREFIX(_lut_pairs)(ptr, idx); }
    inline v_int64 vx_lut_pairs(const int64 * ptr, const int* idx) { return VXPREFIX(_lut_pairs)(ptr, idx); }
    inline v_uint64 vx_lut_pairs(const uint64 * ptr, const int* idx) { return VXPREFIX(_lut_pairs)(ptr, idx); }
#if CV_SIMD_64F || CV_SIMD_SCALABLE_64F
    inline v_float64 vx_lut_pairs(const double* ptr, const int* idx) { return VXPREFIX(_lut_pairs)(ptr, idx); }
#endif
    //! @}

    //! @name Wide LUT of element quads
    //! @{
    //! @brief Load maximum available capacity register contents with array element quads by provided indexes
    inline v_uint8 vx_lut_quads(const uchar* ptr, const int* idx) { return VXPREFIX(_lut_quads)(ptr, idx); }
    inline v_int8 vx_lut_quads(const schar* ptr, const int* idx) { return VXPREFIX(_lut_quads)(ptr, idx); }
    inline v_uint16 vx_lut_quads(const ushort* ptr, const int* idx) { return VXPREFIX(_lut_quads)(ptr, idx); }
    inline v_int16 vx_lut_quads(const short* ptr, const int* idx) { return VXPREFIX(_lut_quads)(ptr, idx); }
    inline v_int32 vx_lut_quads(const int* ptr, const int* idx) { return VXPREFIX(_lut_quads)(ptr, idx); }
    inline v_uint32 vx_lut_quads(const unsigned* ptr, const int* idx) { return VXPREFIX(_lut_quads)(ptr, idx); }
    inline v_float32 vx_lut_quads(const float* ptr, const int* idx) { return VXPREFIX(_lut_quads)(ptr, idx); }
    //! @}

    //! @name Wide load with double expansion
    //! @{
    //! @brief Load maximum available capacity register contents from memory with double expand
    inline v_uint16 vx_load_expand(const uchar * ptr) { return VXPREFIX(_load_expand)(ptr); }
    inline v_int16 vx_load_expand(const schar * ptr) { return VXPREFIX(_load_expand)(ptr); }
    inline v_uint32 vx_load_expand(const ushort * ptr) { return VXPREFIX(_load_expand)(ptr); }
    inline v_int32 vx_load_expand(const short* ptr) { return VXPREFIX(_load_expand)(ptr); }
    inline v_int64 vx_load_expand(const int* ptr) { return VXPREFIX(_load_expand)(ptr); }
    inline v_uint64 vx_load_expand(const unsigned* ptr) { return VXPREFIX(_load_expand)(ptr); }
    inline v_float32 vx_load_expand(const hfloat * ptr) { return VXPREFIX(_load_expand)(ptr); }
    //! @}

    //! @name Wide load with quad expansion
    //! @{
    //! @brief Load maximum available capacity register contents from memory with quad expand
    inline v_uint32 vx_load_expand_q(const uchar * ptr) { return VXPREFIX(_load_expand_q)(ptr); }
    inline v_int32 vx_load_expand_q(const schar * ptr) { return VXPREFIX(_load_expand_q)(ptr); }
    //! @}

    /** @brief SIMD processing state cleanup call */
    inline void vx_cleanup() { VXPREFIX(_cleanup)(); }

#if !CV_SIMD_SCALABLE && !(CV_NEON && !defined(CV_FORCE_SIMD128_CPP))
    // Compatibility layer

    template<typename T> struct VTraits {
        static inline int vlanes() { return T::nlanes; }
        enum { nlanes = T::nlanes, max_nlanes = T::nlanes };
        using lane_type = typename T::lane_type;
    };

    #define OPENCV_HAL_WRAP_BIN_OP_ADDSUB(_Tpvec) \
    inline _Tpvec v_add(const _Tpvec& a, const _Tpvec& b) \
    { \
        return a + b; \
    } \
    inline _Tpvec v_sub(const _Tpvec& a, const _Tpvec& b) \
    { \
        return a - b; \
    } \
    template<typename... Args> \
    inline _Tpvec v_add(const _Tpvec& f1, const _Tpvec& f2, const Args&... vf) { \
        return v_add(f1 + f2, vf...); \
    }
    #define OPENCV_HAL_WRAP_SHIFT_OP(_Tpvec) \
    inline _Tpvec v_shr(const _Tpvec& a, int n) \
    { \
        return a >> n; \
    } \
    inline _Tpvec v_shl(const _Tpvec& a, int n) \
    { \
        return a << n; \
    }

    OPENCV_HAL_WRAP_SHIFT_OP(v_uint16)
    OPENCV_HAL_WRAP_SHIFT_OP(v_uint32)
    OPENCV_HAL_WRAP_SHIFT_OP(v_uint64)
    OPENCV_HAL_WRAP_SHIFT_OP(v_int16)
    OPENCV_HAL_WRAP_SHIFT_OP(v_int32)
    OPENCV_HAL_WRAP_SHIFT_OP(v_int64)
    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_uint8)
    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_uint16)
    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_uint32)
    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_uint64)
    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_int8)
    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_int16)
    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_int32)
    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_int64)
    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_float32)
    #if CV_SIMD_64F
    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_float64)
    #endif
    #if CV_SIMD_WIDTH != 16/*128*/ && CV_SIMD128
    // when we use CV_SIMD128 with 256/512 bit SIMD (e.g. AVX2 or AVX512)
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_uint8x16)
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_uint16x8)
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_uint32x4)
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_uint64x2)
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_int8x16)
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_int16x8)
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_int32x4)
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_int64x2)
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_float32x4)
        OPENCV_HAL_WRAP_SHIFT_OP(v_uint16x8)
        OPENCV_HAL_WRAP_SHIFT_OP(v_uint32x4)
        OPENCV_HAL_WRAP_SHIFT_OP(v_uint64x2)
        OPENCV_HAL_WRAP_SHIFT_OP(v_int16x8)
        OPENCV_HAL_WRAP_SHIFT_OP(v_int32x4)
        OPENCV_HAL_WRAP_SHIFT_OP(v_int64x2)
        #if CV_SIMD_64F
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_float64x2)
        #endif
    #endif
    #if CV_SIMD_WIDTH != 32/*256*/ && CV_SIMD256
    // when we use CV_SIMD256 with 512 bit SIMD (e.g. AVX512)
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_uint8x32)
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_uint16x16)
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_uint32x8)
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_uint64x4)
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_int8x32)
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_int16x16)
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_int32x8)
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_int64x4)
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_float32x8)
        OPENCV_HAL_WRAP_SHIFT_OP(v_uint16x16)
        OPENCV_HAL_WRAP_SHIFT_OP(v_uint32x8)
        OPENCV_HAL_WRAP_SHIFT_OP(v_uint64x4)
        OPENCV_HAL_WRAP_SHIFT_OP(v_int16x16)
        OPENCV_HAL_WRAP_SHIFT_OP(v_int32x8)
        OPENCV_HAL_WRAP_SHIFT_OP(v_int64x4)
        #if CV_SIMD_64F
        OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_float64x4)
        #endif
    #endif

    #define OPENCV_HAL_WRAP_BIN_OP_LOGIC(_Tpvec) \
    inline _Tpvec v_and(const _Tpvec& a, const _Tpvec& b) \
    { \
        return a & b; \
    } \
    inline _Tpvec v_or(const _Tpvec& a, const _Tpvec& b) \
    { \
        return a | b; \
    } \
    inline _Tpvec v_xor(const _Tpvec& a, const _Tpvec& b) \
    { \
        return a ^ b; \
    }

    #define OPENCV_HAL_WRAP_NOT_OP(_Tpvec) \
    inline _Tpvec v_not(const _Tpvec& a) \
    { \
        return ~a; \
    }

    OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_uint8)
    OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_uint16)
    OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_uint32)
    OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_uint64)
    OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_int8)
    OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_int16)
    OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_int32)
    OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_int64)
    OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_float32)
    OPENCV_HAL_WRAP_NOT_OP(v_uint8)
    OPENCV_HAL_WRAP_NOT_OP(v_uint16)
    OPENCV_HAL_WRAP_NOT_OP(v_uint32)
    OPENCV_HAL_WRAP_NOT_OP(v_uint64)
    OPENCV_HAL_WRAP_NOT_OP(v_int8)
    OPENCV_HAL_WRAP_NOT_OP(v_int16)
    OPENCV_HAL_WRAP_NOT_OP(v_int32)
    OPENCV_HAL_WRAP_NOT_OP(v_int64)
    #if CV_SIMD_64F
    OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_float64)
    #endif
    #if CV_SIMD_WIDTH != 16/*128*/ && CV_SIMD128
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_uint8x16)
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_uint16x8)
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_uint32x4)
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_uint64x2)
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_int8x16)
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_int16x8)
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_int32x4)
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_int64x2)
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_float32x4)
        OPENCV_HAL_WRAP_NOT_OP(v_uint8x16)
        OPENCV_HAL_WRAP_NOT_OP(v_uint16x8)
        OPENCV_HAL_WRAP_NOT_OP(v_uint32x4)
        OPENCV_HAL_WRAP_NOT_OP(v_uint64x2)
        OPENCV_HAL_WRAP_NOT_OP(v_int8x16)
        OPENCV_HAL_WRAP_NOT_OP(v_int16x8)
        OPENCV_HAL_WRAP_NOT_OP(v_int32x4)
        OPENCV_HAL_WRAP_NOT_OP(v_int64x2)
        #if CV_SIMD_64F
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_float64x2)
        #endif
    #endif
    #if CV_SIMD_WIDTH != 32/*256*/ && CV_SIMD256
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_uint8x32)
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_uint16x16)
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_uint32x8)
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_uint64x4)
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_int8x32)
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_int16x16)
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_int32x8)
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_int64x4)
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_float32x8)
        OPENCV_HAL_WRAP_NOT_OP(v_uint8x32)
        OPENCV_HAL_WRAP_NOT_OP(v_uint16x16)
        OPENCV_HAL_WRAP_NOT_OP(v_uint32x8)
        OPENCV_HAL_WRAP_NOT_OP(v_uint64x4)
        OPENCV_HAL_WRAP_NOT_OP(v_int8x32)
        OPENCV_HAL_WRAP_NOT_OP(v_int16x16)
        OPENCV_HAL_WRAP_NOT_OP(v_int32x8)
        OPENCV_HAL_WRAP_NOT_OP(v_int64x4)
        #if CV_SIMD_64F
        OPENCV_HAL_WRAP_BIN_OP_LOGIC(v_float64x4)
        #endif
    #endif

    #define OPENCV_HAL_WRAP_BIN_OP_MUL(_Tpvec) \
    inline _Tpvec v_mul(const _Tpvec& a, const _Tpvec& b) \
    { \
        return a * b; \
    } \
    template<typename... Args> \
    inline _Tpvec v_mul(const _Tpvec& f1, const _Tpvec& f2, const Args&... vf) { \
        return v_mul(f1 * f2, vf...); \
    }
    OPENCV_HAL_WRAP_BIN_OP_MUL(v_uint8)
    OPENCV_HAL_WRAP_BIN_OP_MUL(v_int8)
    OPENCV_HAL_WRAP_BIN_OP_MUL(v_uint16)
    OPENCV_HAL_WRAP_BIN_OP_MUL(v_uint32)
    OPENCV_HAL_WRAP_BIN_OP_MUL(v_int16)
    OPENCV_HAL_WRAP_BIN_OP_MUL(v_int32)
    OPENCV_HAL_WRAP_BIN_OP_MUL(v_float32)
    #if CV_SIMD_64F
    OPENCV_HAL_WRAP_BIN_OP_MUL(v_float64)
    #endif
    #if CV_SIMD_WIDTH != 16/*128*/ && CV_SIMD128
        OPENCV_HAL_WRAP_BIN_OP_MUL(v_uint8x16)
        OPENCV_HAL_WRAP_BIN_OP_MUL(v_uint16x8)
        OPENCV_HAL_WRAP_BIN_OP_MUL(v_uint32x4)
        OPENCV_HAL_WRAP_BIN_OP_MUL(v_int8x16)
        OPENCV_HAL_WRAP_BIN_OP_MUL(v_int16x8)
        OPENCV_HAL_WRAP_BIN_OP_MUL(v_int32x4)
        OPENCV_HAL_WRAP_BIN_OP_MUL(v_float32x4)
        #if CV_SIMD_64F
        OPENCV_HAL_WRAP_BIN_OP_MUL(v_float64x2)
        #endif
    #endif
    #if CV_SIMD_WIDTH != 32/*256*/ && CV_SIMD256
        OPENCV_HAL_WRAP_BIN_OP_MUL(v_uint8x32)
        OPENCV_HAL_WRAP_BIN_OP_MUL(v_uint16x16)
        OPENCV_HAL_WRAP_BIN_OP_MUL(v_uint32x8)
        OPENCV_HAL_WRAP_BIN_OP_MUL(v_int8x32)
        OPENCV_HAL_WRAP_BIN_OP_MUL(v_int16x16)
        OPENCV_HAL_WRAP_BIN_OP_MUL(v_int32x8)
        OPENCV_HAL_WRAP_BIN_OP_MUL(v_float32x8)
        #if CV_SIMD_64F
        OPENCV_HAL_WRAP_BIN_OP_MUL(v_float64x4)
        #endif
    #endif

    #define OPENCV_HAL_WRAP_BIN_OP_DIV(_Tpvec) \
    inline _Tpvec v_div(const _Tpvec& a, const _Tpvec& b) \
    { \
        return a / b; \
    }
    OPENCV_HAL_WRAP_BIN_OP_DIV(v_float32)
    #if CV_SIMD_64F
    OPENCV_HAL_WRAP_BIN_OP_DIV(v_float64)
    #endif
    #if CV_SIMD_WIDTH != 16/*128*/ && CV_SIMD128
        OPENCV_HAL_WRAP_BIN_OP_DIV(v_float32x4)
        #if CV_SIMD_64F
        OPENCV_HAL_WRAP_BIN_OP_DIV(v_float64x2)
        #endif
    #endif
    #if CV_SIMD_WIDTH != 32/*256*/ && CV_SIMD256
        OPENCV_HAL_WRAP_BIN_OP_DIV(v_float32x8)
        #if CV_SIMD_64F
        OPENCV_HAL_WRAP_BIN_OP_DIV(v_float64x4)
        #endif
    #endif

    #define OPENCV_HAL_WRAP_CMP_OP(_Tpvec, intrin, op) \
    inline _Tpvec v_##intrin(const _Tpvec& a, const _Tpvec& b) \
    { \
        return a op b; \
    }
    #define OPENCV_HAL_WRAP_EQ_OP(_Tpvec) \
    inline _Tpvec v_eq(const _Tpvec& a, const _Tpvec& b) \
    { \
        return a == b; \
    } \
    inline _Tpvec v_ne(const _Tpvec& a, const _Tpvec& b) \
    { \
        return a != b; \
    }

    #define OPENCV_HAL_WRAP_CMP(_Tpvec) \
    OPENCV_HAL_WRAP_CMP_OP(_Tpvec, eq, ==) \
    OPENCV_HAL_WRAP_CMP_OP(_Tpvec, ne, !=) \
    OPENCV_HAL_WRAP_CMP_OP(_Tpvec, lt, <) \
    OPENCV_HAL_WRAP_CMP_OP(_Tpvec, gt, >) \
    OPENCV_HAL_WRAP_CMP_OP(_Tpvec, le, <=) \
    OPENCV_HAL_WRAP_CMP_OP(_Tpvec, ge, >=)

    OPENCV_HAL_WRAP_CMP(v_uint8)
    OPENCV_HAL_WRAP_CMP(v_uint16)
    OPENCV_HAL_WRAP_CMP(v_uint32)
    OPENCV_HAL_WRAP_EQ_OP(v_uint64)
    OPENCV_HAL_WRAP_CMP(v_int8)
    OPENCV_HAL_WRAP_CMP(v_int16)
    OPENCV_HAL_WRAP_CMP(v_int32)
    OPENCV_HAL_WRAP_EQ_OP(v_int64)
    OPENCV_HAL_WRAP_CMP(v_float32)
    #if CV_SIMD_64F
    OPENCV_HAL_WRAP_CMP(v_float64)
    #endif
    #if CV_SIMD_WIDTH != 16/*128*/ && CV_SIMD128
        OPENCV_HAL_WRAP_CMP(v_uint8x16)
        OPENCV_HAL_WRAP_CMP(v_uint16x8)
        OPENCV_HAL_WRAP_CMP(v_uint32x4)
        OPENCV_HAL_WRAP_EQ_OP(v_uint64x2)
        OPENCV_HAL_WRAP_CMP(v_int8x16)
        OPENCV_HAL_WRAP_CMP(v_int16x8)
        OPENCV_HAL_WRAP_CMP(v_int32x4)
        OPENCV_HAL_WRAP_EQ_OP(v_int64x2)
        OPENCV_HAL_WRAP_CMP(v_float32x4)
        #if CV_SIMD_64F
        OPENCV_HAL_WRAP_CMP(v_float64x2)
        #endif
    #endif
    #if CV_SIMD_WIDTH != 32/*256*/ && CV_SIMD256
        OPENCV_HAL_WRAP_CMP(v_uint8x32)
        OPENCV_HAL_WRAP_CMP(v_uint16x16)
        OPENCV_HAL_WRAP_CMP(v_uint32x8)
        OPENCV_HAL_WRAP_EQ_OP(v_uint64x4)
        OPENCV_HAL_WRAP_CMP(v_int8x32)
        OPENCV_HAL_WRAP_CMP(v_int16x16)
        OPENCV_HAL_WRAP_CMP(v_int32x8)
        OPENCV_HAL_WRAP_EQ_OP(v_int64x4)
        OPENCV_HAL_WRAP_CMP(v_float32x8)
        #if CV_SIMD_64F
        OPENCV_HAL_WRAP_CMP(v_float64x4)
        #endif
    #endif

    //////////// get0 ////////////
    #define OPENCV_HAL_WRAP_GRT0(_Tpvec) \
    inline typename VTraits<_Tpvec>::lane_type v_get0(const _Tpvec& v) \
    { \
        return v.get0(); \
    }

    OPENCV_HAL_WRAP_GRT0(v_uint8)
    OPENCV_HAL_WRAP_GRT0(v_int8)
    OPENCV_HAL_WRAP_GRT0(v_uint16)
    OPENCV_HAL_WRAP_GRT0(v_int16)
    OPENCV_HAL_WRAP_GRT0(v_uint32)
    OPENCV_HAL_WRAP_GRT0(v_int32)
    OPENCV_HAL_WRAP_GRT0(v_uint64)
    OPENCV_HAL_WRAP_GRT0(v_int64)
    OPENCV_HAL_WRAP_GRT0(v_float32)
    #if CV_SIMD_64F
    OPENCV_HAL_WRAP_GRT0(v_float64)
    #endif
    #if CV_SIMD_WIDTH != 16/*128*/ && CV_SIMD128
        OPENCV_HAL_WRAP_GRT0(v_uint8x16)
        OPENCV_HAL_WRAP_GRT0(v_uint16x8)
        OPENCV_HAL_WRAP_GRT0(v_uint32x4)
        OPENCV_HAL_WRAP_GRT0(v_uint64x2)
        OPENCV_HAL_WRAP_GRT0(v_int8x16)
        OPENCV_HAL_WRAP_GRT0(v_int16x8)
        OPENCV_HAL_WRAP_GRT0(v_int32x4)
        OPENCV_HAL_WRAP_GRT0(v_int64x2)
        OPENCV_HAL_WRAP_GRT0(v_float32x4)
        #if CV_SIMD_64F
        OPENCV_HAL_WRAP_GRT0(v_float64x2)
        #endif
    #endif
    #if CV_SIMD_WIDTH != 32/*256*/ && CV_SIMD256
        OPENCV_HAL_WRAP_GRT0(v_uint8x32)
        OPENCV_HAL_WRAP_GRT0(v_uint16x16)
        OPENCV_HAL_WRAP_GRT0(v_uint32x8)
        OPENCV_HAL_WRAP_GRT0(v_uint64x4)
        OPENCV_HAL_WRAP_GRT0(v_int8x32)
        OPENCV_HAL_WRAP_GRT0(v_int16x16)
        OPENCV_HAL_WRAP_GRT0(v_int32x8)
        OPENCV_HAL_WRAP_GRT0(v_int64x4)
        OPENCV_HAL_WRAP_GRT0(v_float32x8)
        #if CV_SIMD_64F
        OPENCV_HAL_WRAP_GRT0(v_float64x4)
        #endif
    #endif

    #define OPENCV_HAL_WRAP_EXTRACT(_Tpvec) \
    inline typename VTraits<_Tpvec>::lane_type v_extract_highest(const _Tpvec& v) \
    { \
        return v_extract_n<VTraits<_Tpvec>::nlanes-1>(v); \
    }

    OPENCV_HAL_WRAP_EXTRACT(v_uint8)
    OPENCV_HAL_WRAP_EXTRACT(v_int8)
    OPENCV_HAL_WRAP_EXTRACT(v_uint16)
    OPENCV_HAL_WRAP_EXTRACT(v_int16)
    OPENCV_HAL_WRAP_EXTRACT(v_uint32)
    OPENCV_HAL_WRAP_EXTRACT(v_int32)
    OPENCV_HAL_WRAP_EXTRACT(v_uint64)
    OPENCV_HAL_WRAP_EXTRACT(v_int64)
    OPENCV_HAL_WRAP_EXTRACT(v_float32)
    #if CV_SIMD_64F
    OPENCV_HAL_WRAP_EXTRACT(v_float64)
    #endif
    #if CV_SIMD_WIDTH != 16/*128*/ && CV_SIMD128
        OPENCV_HAL_WRAP_EXTRACT(v_uint8x16)
        OPENCV_HAL_WRAP_EXTRACT(v_uint16x8)
        OPENCV_HAL_WRAP_EXTRACT(v_uint32x4)
        OPENCV_HAL_WRAP_EXTRACT(v_uint64x2)
        OPENCV_HAL_WRAP_EXTRACT(v_int8x16)
        OPENCV_HAL_WRAP_EXTRACT(v_int16x8)
        OPENCV_HAL_WRAP_EXTRACT(v_int32x4)
        OPENCV_HAL_WRAP_EXTRACT(v_int64x2)
        OPENCV_HAL_WRAP_EXTRACT(v_float32x4)
        #if CV_SIMD_64F
        OPENCV_HAL_WRAP_EXTRACT(v_float64x2)
        #endif
    #endif
    #if CV_SIMD_WIDTH != 32/*256*/ && CV_SIMD256
        OPENCV_HAL_WRAP_EXTRACT(v_uint8x32)
        OPENCV_HAL_WRAP_EXTRACT(v_uint16x16)
        OPENCV_HAL_WRAP_EXTRACT(v_uint32x8)
        OPENCV_HAL_WRAP_EXTRACT(v_uint64x4)
        OPENCV_HAL_WRAP_EXTRACT(v_int8x32)
        OPENCV_HAL_WRAP_EXTRACT(v_int16x16)
        OPENCV_HAL_WRAP_EXTRACT(v_int32x8)
        OPENCV_HAL_WRAP_EXTRACT(v_int64x4)
        OPENCV_HAL_WRAP_EXTRACT(v_float32x8)
        #if CV_SIMD_64F
        OPENCV_HAL_WRAP_EXTRACT(v_float64x4)
        #endif
    #endif

    #define OPENCV_HAL_WRAP_BROADCAST(_Tpvec) \
    inline _Tpvec v_broadcast_highest(const _Tpvec& v) \
    { \
        return v_broadcast_element<VTraits<_Tpvec>::nlanes-1>(v); \
    }

    OPENCV_HAL_WRAP_BROADCAST(v_uint32)
    OPENCV_HAL_WRAP_BROADCAST(v_int32)
    OPENCV_HAL_WRAP_BROADCAST(v_float32)
    #if CV_SIMD_WIDTH != 16/*128*/ && CV_SIMD128
        OPENCV_HAL_WRAP_BROADCAST(v_uint32x4)
        OPENCV_HAL_WRAP_BROADCAST(v_int32x4)
        OPENCV_HAL_WRAP_BROADCAST(v_float32x4)
    #endif
    #if CV_SIMD_WIDTH != 32/*256*/ && CV_SIMD256
        OPENCV_HAL_WRAP_BROADCAST(v_uint32x8)
        OPENCV_HAL_WRAP_BROADCAST(v_int32x8)
        OPENCV_HAL_WRAP_BROADCAST(v_float32x8)
    #endif

#endif //!CV_SIMD_SCALABLE

#if (CV_NEON /* || CV_others */) && !defined(CV_FORCE_SIMD128_CPP)
// Compatibility layer for the backend that cleaned up.
    #define OPENCV_HAL_WRAP_BIN_OP_ADDSUB(_Tpvec) \
    template<typename... Args> \
    inline _Tpvec v_add(const _Tpvec& f1, const _Tpvec& f2, const Args&... vf) { \
        return v_add(v_add(f1, f2), vf...); \
    }

    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_uint8)
    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_uint16)
    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_uint32)
    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_uint64)
    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_int8)
    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_int16)
    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_int32)
    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_int64)
    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_float32)
    #if CV_SIMD_64F
    OPENCV_HAL_WRAP_BIN_OP_ADDSUB(v_float64)
    #endif

    #define OPENCV_HAL_WRAP_BIN_OP_MUL(_Tpvec) \
    template<typename... Args> \
    inline _Tpvec v_mul(const _Tpvec& f1, const _Tpvec& f2, const Args&... vf) { \
        return v_mul(v_mul(f1, f2), vf...); \
    }
    OPENCV_HAL_WRAP_BIN_OP_MUL(v_uint8)
    OPENCV_HAL_WRAP_BIN_OP_MUL(v_int8)
    OPENCV_HAL_WRAP_BIN_OP_MUL(v_uint16)
    OPENCV_HAL_WRAP_BIN_OP_MUL(v_uint32)
    OPENCV_HAL_WRAP_BIN_OP_MUL(v_int16)
    OPENCV_HAL_WRAP_BIN_OP_MUL(v_int32)
    OPENCV_HAL_WRAP_BIN_OP_MUL(v_float32)
    #if CV_SIMD_64F
    OPENCV_HAL_WRAP_BIN_OP_MUL(v_float64)
    #endif

    #define OPENCV_HAL_WRAP_EXTRACT(_Tpvec) \
    inline typename VTraits<_Tpvec>::lane_type v_extract_highest(const _Tpvec& v) \
    { \
        return v_extract_n<VTraits<_Tpvec>::nlanes-1>(v); \
    }

    OPENCV_HAL_WRAP_EXTRACT(v_uint8)
    OPENCV_HAL_WRAP_EXTRACT(v_int8)
    OPENCV_HAL_WRAP_EXTRACT(v_uint16)
    OPENCV_HAL_WRAP_EXTRACT(v_int16)
    OPENCV_HAL_WRAP_EXTRACT(v_uint32)
    OPENCV_HAL_WRAP_EXTRACT(v_int32)
    OPENCV_HAL_WRAP_EXTRACT(v_uint64)
    OPENCV_HAL_WRAP_EXTRACT(v_int64)
    OPENCV_HAL_WRAP_EXTRACT(v_float32)
    #if CV_SIMD_64F
    OPENCV_HAL_WRAP_EXTRACT(v_float64)
    #endif

    #define OPENCV_HAL_WRAP_BROADCAST(_Tpvec) \
    inline _Tpvec v_broadcast_highest(const _Tpvec& v) \
    { \
        return v_broadcast_element<VTraits<_Tpvec>::nlanes-1>(v); \
    }

    OPENCV_HAL_WRAP_BROADCAST(v_uint32)
    OPENCV_HAL_WRAP_BROADCAST(v_int32)
    OPENCV_HAL_WRAP_BROADCAST(v_float32)

#endif //CV_NEON

//! @cond IGNORED

    // backward compatibility
    template<typename _Tp, typename _Tvec> static inline
    void vx_store(_Tp* dst, const _Tvec& v) { return v_store(dst, v); }
    // backward compatibility
    template<typename _Tp, typename _Tvec> static inline
    void vx_store_aligned(_Tp* dst, const _Tvec& v) { return v_store_aligned(dst, v); }

//! @endcond

//! @name Exponential
//! @{
#if defined(CV_SIMD_FP16) && CV_SIMD_FP16
    // Implementation is the same as float32 vector.
    inline v_float16 v_exp(const v_float16 &x) {
        const v_float16 _vexp_lo_f16 = vx_setall_f16(-10.7421875f);
        const v_float16 _vexp_hi_f16 = vx_setall_f16(11.f);
        const v_float16 _vexp_half_fp16 = vx_setall_f16(0.5f);
        const v_float16 _vexp_one_fp16 = vx_setall_f16(1.f);
        const v_float16 _vexp_LOG2EF_f16 = vx_setall_f16(1.44269504088896341f);
        const v_float16 _vexp_C1_f16 = vx_setall_f16(-6.93359375E-1f);
        const v_float16 _vexp_C2_f16 = vx_setall_f16(2.12194440E-4f);
        const v_float16 _vexp_p0_f16 = vx_setall_f16(1.9875691500E-4f);
        const v_float16 _vexp_p1_f16 = vx_setall_f16(1.3981999507E-3f);
        const v_float16 _vexp_p2_f16 = vx_setall_f16(8.3334519073E-3f);
        const v_float16 _vexp_p3_f16 = vx_setall_f16(4.1665795894E-2f);
        const v_float16 _vexp_p4_f16 = vx_setall_f16(1.6666665459E-1f);
        const v_float16 _vexp_p5_f16 = vx_setall_f16(5.0000001201E-1f);
        const v_int16 _vexp_bias_s16 = vx_setall_s16(0xf);

        v_float16 _vexp_, _vexp_x, _vexp_y, _vexp_xx;
        v_int16 _vexp_mm;

        // compute exponential of x
        _vexp_x = v_max(x, _vexp_lo_f16);
        _vexp_x = v_min(_vexp_x, _vexp_hi_f16);

        _vexp_ = v_fma(_vexp_x, _vexp_LOG2EF_f16, _vexp_half_fp16);
        _vexp_mm = v_floor(_vexp_);
        _vexp_ = v_cvt_f16(_vexp_mm);
        _vexp_mm = v_add(_vexp_mm, _vexp_bias_s16);
        _vexp_mm = v_shl(_vexp_mm, 10);

        _vexp_x = v_fma(_vexp_, _vexp_C1_f16, _vexp_x);
        _vexp_x = v_fma(_vexp_, _vexp_C2_f16, _vexp_x);
        _vexp_xx = v_mul(_vexp_x, _vexp_x);

        _vexp_y = v_fma(_vexp_x, _vexp_p0_f16, _vexp_p1_f16);
        _vexp_y = v_fma(_vexp_y, _vexp_x, _vexp_p2_f16);
        _vexp_y = v_fma(_vexp_y, _vexp_x, _vexp_p3_f16);
        _vexp_y = v_fma(_vexp_y, _vexp_x, _vexp_p4_f16);
        _vexp_y = v_fma(_vexp_y, _vexp_x, _vexp_p5_f16);

        _vexp_y = v_fma(_vexp_y, _vexp_xx, _vexp_x);
        _vexp_y = v_add(_vexp_y, _vexp_one_fp16);
        _vexp_y = v_mul(_vexp_y, v_reinterpret_as_f16(_vexp_mm));

        // exp(NAN) -> NAN
        v_float16 mask_nan = v_ne(x, x);
        return v_select(mask_nan, v_reinterpret_as_f16(vx_setall_s16(0x7e00)), _vexp_y);
    }
#endif


    /* Universal intrinsic implementation of exponential function for float32 vectors.

       Inspired by Intel Approximate Math library, and based on the
       corresponding algorithms of the cephes math library
    */

    /* Copyright (C) 2011  Julien Pommier

      This software is provided 'as-is', without any express or implied
      warranty.  In no event will the authors be held liable for any damages
      arising from the use of this software.

      Permission is granted to anyone to use this software for any purpose,
      including commercial applications, and to alter it and redistribute it
      freely, subject to the following restrictions:

      1. The origin of this software must not be misrepresented; you must not
         claim that you wrote the original software. If you use this software
         in a product, an acknowledgment in the product documentation would be
         appreciated but is not required.
      2. Altered source versions must be plainly marked as such, and must not be
         misrepresented as being the original software.
      3. This notice may not be removed or altered from any source distribution.

      (this is the zlib license)
    */

    // Reference: https://github.com/aff3ct/MIPP/blob/f3cd53611cc3a9688e30632b8c1d13eccb701aa7/include/math/neon_mathfun.hxx#L119-L192
    inline v_float32 v_exp(const v_float32 &x) {
        const v_float32 _vexp_lo_f32 = vx_setall_f32(-88.3762626647949f);
        const v_float32 _vexp_hi_f32 = vx_setall_f32(89.f);
        const v_float32 _vexp_half_fp32 = vx_setall_f32(0.5f);
        const v_float32 _vexp_one_fp32 = vx_setall_f32(1.f);
        const v_float32 _vexp_LOG2EF_f32 = vx_setall_f32(1.44269504088896341f);
        const v_float32 _vexp_C1_f32 = vx_setall_f32(-6.93359375E-1f);
        const v_float32 _vexp_C2_f32 = vx_setall_f32(2.12194440E-4f);
        const v_float32 _vexp_p0_f32 = vx_setall_f32(1.9875691500E-4f);
        const v_float32 _vexp_p1_f32 = vx_setall_f32(1.3981999507E-3f);
        const v_float32 _vexp_p2_f32 = vx_setall_f32(8.3334519073E-3f);
        const v_float32 _vexp_p3_f32 = vx_setall_f32(4.1665795894E-2f);
        const v_float32 _vexp_p4_f32 = vx_setall_f32(1.6666665459E-1f);
        const v_float32 _vexp_p5_f32 = vx_setall_f32(5.0000001201E-1f);
        const v_int32 _vexp_bias_s32 = vx_setall_s32(0x7f);

        v_float32 _vexp_, _vexp_x, _vexp_y, _vexp_xx;
        v_int32 _vexp_mm;

        // compute exponential of x
        _vexp_x = v_max(x, _vexp_lo_f32);
        _vexp_x = v_min(_vexp_x, _vexp_hi_f32);

        _vexp_ = v_fma(_vexp_x, _vexp_LOG2EF_f32, _vexp_half_fp32);
        _vexp_mm = v_floor(_vexp_);
        _vexp_ = v_cvt_f32(_vexp_mm);
        _vexp_mm = v_add(_vexp_mm, _vexp_bias_s32);
        _vexp_mm = v_shl(_vexp_mm, 23);

        _vexp_x = v_fma(_vexp_, _vexp_C1_f32, _vexp_x);
        _vexp_x = v_fma(_vexp_, _vexp_C2_f32, _vexp_x);
        _vexp_xx = v_mul(_vexp_x, _vexp_x);

        _vexp_y = v_fma(_vexp_x, _vexp_p0_f32, _vexp_p1_f32);
        _vexp_y = v_fma(_vexp_y, _vexp_x, _vexp_p2_f32);
        _vexp_y = v_fma(_vexp_y, _vexp_x, _vexp_p3_f32);
        _vexp_y = v_fma(_vexp_y, _vexp_x, _vexp_p4_f32);
        _vexp_y = v_fma(_vexp_y, _vexp_x, _vexp_p5_f32);

        _vexp_y = v_fma(_vexp_y, _vexp_xx, _vexp_x);
        _vexp_y = v_add(_vexp_y, _vexp_one_fp32);
        _vexp_y = v_mul(_vexp_y, v_reinterpret_as_f32(_vexp_mm));

        // exp(NAN) -> NAN
        v_float32 mask_nan = v_ne(x, x);
        return v_select(mask_nan, v_reinterpret_as_f32(vx_setall_s32(0x7fc00000)), _vexp_y);
    }

#if CV_SIMD_64F || CV_SIMD_SCALABLE_64F

    /*
     * Universal intrinsic implementation of exponential function for float64 vectors.
     * The implementation is inspired by the approach outlined from https://root.cern.ch/root/html608/exp_8h_source.html
     */

    /*
     * exp.h
     * The basic idea is to exploit Pade polynomials.
     * A lot of ideas were inspired by the cephes math library (by Stephen L. Moshier
     * moshier@na-net.ornl.gov) as well as actual code.
     * The Cephes library can be found here:  http://www.netlib.org/cephes/
     *
     *  Created on: Jun 23, 2012
     *      Author: Danilo Piparo, Thomas Hauth, Vincenzo Innocente
     */

    /*
     * VDT is free software: you can redistribute it and/or modify
     * it under the terms of the GNU Lesser Public License as published by
     * the Free Software Foundation, either version 3 of the License, or
     * (at your option) any later version.
     *
     * This program is distributed in the hope that it will be useful,
     * but WITHOUT ANY WARRANTY; without even the implied warranty of
     * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     * GNU Lesser Public License for more details.
     *
     * You should have received a copy of the GNU Lesser Public License
     * along with this program.  If not, see <http://www.gnu.org/licenses/>.
     */

    inline v_float64 v_exp(const v_float64 &x) {
        const v_float64 _vexp_lo_f64 = vx_setall_f64(-709.43613930310391424428);
        const v_float64 _vexp_hi_f64 = vx_setall_f64(710.);
        const v_float64 _vexp_half_f64 = vx_setall_f64(0.5);
        const v_float64 _vexp_one_f64 = vx_setall_f64(1.0);
        const v_float64 _vexp_two_f64 = vx_setall_f64(2.0);
        const v_float64 _vexp_LOG2EF_f64 = vx_setall_f64(1.44269504088896340736);
        const v_float64 _vexp_C1_f64 = vx_setall_f64(-6.93145751953125E-1);
        const v_float64 _vexp_C2_f64 = vx_setall_f64(-1.42860682030941723212E-6);
        const v_float64 _vexp_p0_f64 = vx_setall_f64(1.26177193074810590878E-4);
        const v_float64 _vexp_p1_f64 = vx_setall_f64(3.02994407707441961300E-2);
        const v_float64 _vexp_p2_f64 = vx_setall_f64(9.99999999999999999910E-1);
        const v_float64 _vexp_q0_f64 = vx_setall_f64(3.00198505138664455042E-6);
        const v_float64 _vexp_q1_f64 = vx_setall_f64(2.52448340349684104192E-3);
        const v_float64 _vexp_q2_f64 = vx_setall_f64(2.27265548208155028766E-1);
        const v_float64 _vexp_q3_f64 = vx_setall_f64(2.00000000000000000009E0);
        const v_int64 _vexp_bias_s64 = vx_setall_s64(0x3ff);

        v_float64 _vexp_, _vexp_x, _vexp_y, _vexp_z, _vexp_xx;
        v_int64 _vexp_mm;

        // compute exponential of x
        _vexp_x = v_max(x, _vexp_lo_f64);
        _vexp_x = v_min(_vexp_x, _vexp_hi_f64);

        _vexp_ = v_fma(_vexp_x, _vexp_LOG2EF_f64, _vexp_half_f64);
        _vexp_mm = v_expand_low(v_floor(_vexp_));
        _vexp_ = v_cvt_f64(_vexp_mm);
        _vexp_mm = v_add(_vexp_mm, _vexp_bias_s64);
        _vexp_mm = v_shl(_vexp_mm, 52);

        _vexp_x = v_fma(_vexp_, _vexp_C1_f64, _vexp_x);
        _vexp_x = v_fma(_vexp_, _vexp_C2_f64, _vexp_x);
        _vexp_xx = v_mul(_vexp_x, _vexp_x);

        _vexp_y = v_fma(_vexp_xx, _vexp_p0_f64, _vexp_p1_f64);
        _vexp_y = v_fma(_vexp_y, _vexp_xx, _vexp_p2_f64);
        _vexp_y = v_mul(_vexp_y, _vexp_x);

        _vexp_z = v_fma(_vexp_xx, _vexp_q0_f64, _vexp_q1_f64);
        _vexp_z = v_fma(_vexp_xx, _vexp_z, _vexp_q2_f64);
        _vexp_z = v_fma(_vexp_xx, _vexp_z, _vexp_q3_f64);

        _vexp_z = v_div(_vexp_y, v_sub(_vexp_z, _vexp_y));
        _vexp_z = v_fma(_vexp_two_f64, _vexp_z, _vexp_one_f64);
        _vexp_z = v_mul(_vexp_z, v_reinterpret_as_f64(_vexp_mm));

        // exp(NAN) -> NAN
        v_float64 mask_nan = v_ne(x, x);
        return v_select(mask_nan, v_reinterpret_as_f64(vx_setall_s64(0x7FF8000000000000)), _vexp_z);
    }
#endif
//! @}

//! @}
    #undef VXPREFIX
} // namespace


#ifndef CV_SIMD_FP16
#define CV_SIMD_FP16 0  //!< Defined to 1 on native support of operations with float16x8_t / float16x16_t (SIMD256) types
#endif

#ifndef CV_SIMD
#define CV_SIMD 0
#endif

#include "simd_utils.impl.hpp"

#ifndef CV_DOXYGEN
CV_CPU_OPTIMIZATION_HAL_NAMESPACE_END
#endif

} // cv::

//! @endcond

#if defined(__GNUC__) && __GNUC__ == 12
#pragma GCC diagnostic pop
#endif

#endif
