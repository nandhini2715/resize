// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html

#include "precomp.hpp"
#include "try_cpu_features.simd_declarations.hpp"

#if (defined __aarch64__ || defined __arm__) && defined __ARM_NEON
#define CV_CPU_ARM_NEON_ONLY(foo, default_value) foo
#else
#define CV_CPU_ARM_NEON_ONLY(foo, default_value) default_value
#endif

namespace cv
{

bool haveCpuFeatureFp16()
{
    return CV_CPU_ARM_NEON_ONLY(opt_FP16::haveCpuFeatureFp16_(), false);
}

bool haveCpuFeatureNeon()
{
#ifdef __ARM_NEON
    return CV_CPU_ARM_NEON_ONLY(opt_NEON::haveCpuFeatureNeon_(), false);
#else
    return false;
#endif
}

bool haveCpuFeatureDotProd()
{
#ifdef CV_CPU_DISPATCH_COMPILE_NEON_DOTPROD
    return CV_CPU_ARM_NEON_ONLY(opt_NEON_DOTPROD::haveCpuFeatureDotProd_(), false);
#else
    return false;
#endif
}

bool haveCpuFeatureFp16SIMD()
{
#ifdef CV_CPU_DISPATCH_COMPILE_FP16_SIMD
    return CV_CPU_ARM_NEON_ONLY(opt_FP16_SIMD::haveCpuFeatureFp16SIMD_(), false);
#else
    return false;
#endif
}

bool haveCpuFeatureBf16SIMD()
{
#ifdef CV_CPU_DISPATCH_COMPILE_BF16_SIMD
    return CV_CPU_ARM_NEON_ONLY(opt_BF16_SIMD::haveCpuFeatureBf16SIMD_(), false);
#else
    return false;
#endif
}

}
