#pragma once
#include <cstdint>

struct xoshiro256ss_state {
	uint64_t s[4];
};

using xorshift_state = xoshiro256ss_state;
