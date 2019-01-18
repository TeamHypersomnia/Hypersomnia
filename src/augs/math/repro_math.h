#pragma once
#include "augs/build_settings/compiler_defines.h"

#define USE_STREFLOP 0

#if FORCE_DISABLE_STREFLOP
#undef USE_STREFLOP
#define USE_STREFLOP 0
#endif

#if USE_STREFLOP
#include "3rdparty/streflop/streflop.h"
namespace repro = streflop;

namespace streflop_libm {
#ifdef LIBM_COMPILING_FLT32
	extern void __sincosf (Simple x, Simple *sinx, Simple *cosx);
#endif
}
#else
#include <cmath>
namespace repro = std;
#endif

#if USE_STREFLOP
namespace streflop {
#ifdef LIBM_COMPILING_FLT32
	FORCE_INLINE void sincosf(Simple x, Simple& sinx, Simple& cosx) {
		streflop_libm::__sincosf(x, &sinx, &cosx);
	}
#endif
}
#else
namespace std {
	FORCE_INLINE void sincosf(float x, float& sinx, float& cosx) {
		sinx = static_cast<float>(sin(x));
		cosx = static_cast<float>(cos(x));
	}
}
#endif

