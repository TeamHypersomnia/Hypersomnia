#pragma once
#include "3rdparty/streflop/streflop.h"
#include "augs/build_settings/compiler_defines.h"

namespace repro = streflop;

namespace streflop_libm {
#ifdef LIBM_COMPILING_FLT32
	extern void __sincosf (Simple x, Simple *sinx, Simple *cosx);
#endif
}

namespace streflop {
#ifdef LIBM_COMPILING_FLT32
	FORCE_INLINE void sincosf(Simple x, Simple& sinx, Simple& cosx) {
		streflop_libm::__sincosf(x, &sinx, &cosx);
	}
#endif
}
