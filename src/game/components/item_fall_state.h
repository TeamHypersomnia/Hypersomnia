#pragma once
#include <cstdint>
#include "augs/pad_bytes.h"
#include "augs/misc/timing/stepped_timing.h"

inline constexpr uint8_t NO_SOUND_VARIATION = 0xff;

/*
	Items thrown or dropped explicitly, and shells ejected from guns, hit the floor a few times before they come to rest -
	see item_falling.h. when_landed is when they came to rest after the last hit.
	sound_variation picks the variation of the floor hit sounds, or a random one if NO_SOUND_VARIATION.
	hops_like_shell makes an item hop off the floor like shells do - see start_falling_like_unmounted_magazine.
*/

struct item_fall_state {
	// GEN INTROSPECTOR struct item_fall_state
	augs::stepped_timestamp when_landed;
	augs::stepped_timestamp when_started_falling;
	augs::stepped_timestamp when_hop_started;
	real32 hop_duration_secs = 0.f;
	real32 hop_height = 1.f;
	uint8_t floor_hits_left = 0;
	uint8_t floor_hits_done = 0;
	bool thrown_up = false;
	bool thrown_melee = false;
	uint8_t sound_variation = NO_SOUND_VARIATION;
	bool hops_like_shell = false;
	pad_bytes<2> pad;
	// END GEN INTROSPECTOR

	bool is_in_the_air() const {
		return when_started_falling.was_set() && floor_hits_done == 0;
	}
};
