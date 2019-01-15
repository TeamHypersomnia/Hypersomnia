#include "augs/math/repro_math.h"

#include "augs/log.h"
#include "augs/ensure.h"

#if !defined(STREFLOP_SSE)
#error "STREFLOP_SSE was not defined."
#endif

#include "fp_consistency_tests.h"

static_assert(std::is_same_v<repro::Simple, float>);

void setup_float_flags() {
	repro::fesetround(repro::FE_TONEAREST);
	repro::streflop_init<repro::Simple>();
}

void ensure_float_flags_hold() {
	ensure_eq(repro::fegetround(), repro::FE_TONEAREST);
}

bool perform_float_consistency_tests() {
	ensure_float_flags_hold();

	repro::sinf(32.f);
	return true;
}
