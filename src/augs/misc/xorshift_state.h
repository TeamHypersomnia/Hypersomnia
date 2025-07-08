#pragma once
#include <cstdint>

struct xoshiro256ss_state {
	// GEN INTROSPECTOR struct xoshiro256ss_state
	uint64_t s_0 = 0;
	uint64_t s_1 = 0;
	uint64_t s_2 = 0;
	uint64_t s_3 = 0;
	// END GEN INTROSPECTOR
};

using xorshift_state = xoshiro256ss_state;
