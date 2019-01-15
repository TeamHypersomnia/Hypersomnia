#include "augs/math/repro_math.h"

#include "augs/log.h"
#include "augs/ensure.h"

#if PLATFORM_UNIX
#include <fpu_control.h>
#endif

#include "fp_consistency_tests.h"

static_assert(std::is_same_v<repro::Simple, float>);

void setup_float_flags() {
	repro::fesetround(repro::FE_TONEAREST);
	repro::streflop_init<repro::Simple>();

#if PLATFORM_WINDOWS
	_controlfp(_DN_SAVE, _MCW_DN);
#elif PLATFORM_UNIX

#endif
}

void ensure_float_flags_hold() {
	ensure_eq(repro::fegetround(), repro::FE_TONEAREST);

#if PLATFORM_WINDOWS
	ensure_eq(_controlfp(0, 0) & _MCW_DN, _DN_SAVE);
#elif PLATFORM_UNIX

#endif
}

bool perform_float_consistency_tests() {
	ensure_float_flags_hold();

	repro::sinf(32.f);
	return true;
}
