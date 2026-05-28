/* See the import.pl script for potential modifications */
/* e_sqrtf.c -- Simple version of e_sqrt.c.
 * Conversion to Simple by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

#if defined(LIBM_SCCS) && !defined(lint)
static char rcsid[] = "$NetBSD: e_sqrtf.c,v 1.4f 1995/05/10 20:46:19 jtc Exp $";
#endif

#include "SMath.h"
#include "math_private.h"

#if defined(_MSC_VER) && !defined(__clang__) && !defined(__GNUC__)
#include <xmmintrin.h>
#endif

namespace streflop_libm {
	/*
		IEEE 754 mandates correctly-rounded sqrt, so hardware sqrt
		is bit-exact deterministic across all target platforms
		(x64 sqrtss, ARM64 fsqrt, WASM f32.sqrt) under the project's
		-ffp-model=strict / /fp:strict flags.
		Streflop's libm header trampoline shadows <cmath>, so we
		emit the hardware op directly via a builtin / SSE intrinsic
		instead of going through std::sqrt.
	*/
	Simple __ieee754_sqrtf(Simple x)
	{
#if defined(__clang__) || defined(__GNUC__)
		return __builtin_sqrtf(x);
#elif defined(_MSC_VER)
		return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(x)));
#else
#error "unsupported compiler: no portable correctly-rounded sqrtf available"
#endif
	}
}
