#pragma once
#include <array>
#include "augs/math/declare_math.h"
#include "augs/math/vec2.h"
#include "augs/pad_bytes.h"
#include "augs/misc/constant_size_vector.h"
#include "augs/misc/timing/stepped_timing.h"
#include "game/components/item_fall_state.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"

/*
	Remnants kept until evicted - like shells - don't expire after their lifetime_secs,
	but only once there are too many of them, oldest first - see remnant_system.
	Either way they shrink for start_shrinking_when_remaining_ms before disappearing.

	floor_hit_sounds play in order as they hit the floor after being ejected.
	roll_sound plays only as they're kicked into rolling - by characters walking into them.
	ejection_smoke_size_mult scales the smoke of the cartridge they're ejected from - bigger calibers smoke more,
	in bigger particles and longer - see SHELL_SMOKE_PARTICLE_SIZE_PER_SIZE.
	floor_hit_variations_by_height orders the variations of the sounds from the lowest falls to the highest.
	Remnants that don't roll - like shells too heavy for it - get only pushed along and nudged away, never rolled sideways.

	pending_kick is the direction a dynamic body bumping into the remnant kicked it in, until remnant_system handles it.
	seed randomizes everything about a shell - see spawn_shell - and so does num_kicks every next kick, however late it comes.
*/

namespace invariants {
	struct remnant {
		// GEN INTROSPECTOR struct invariants::remnant
		real32 lifetime_secs = 2.f;
		real32 start_shrinking_when_remaining_ms = 10.0f;
		particle_effect_input trace_particles;
		std::array<sound_effect_input, 2> floor_hit_sounds;
		sound_effect_input roll_sound;
		augs::constant_size_vector<uint8_t, 32> floor_hit_variations_by_height;
		real32 ejection_smoke_size_mult = 1.f;
		bool kept_until_evicted = false;
		bool rolls = true;
		pad_bytes<2> pad;
		// END GEN INTROSPECTOR
	};
}

namespace components {
	struct remnant {
		// GEN INTROSPECTOR struct components::remnant
		real32 last_size_mult = 1.f;
		augs::stepped_timestamp when_evicted;
		item_fall_state fall;
		vec2 pending_kick;
		augs::stepped_timestamp when_kicked;
		uint32_t num_kicks = 0;
		uint32_t seed = 0;
		entity_id ejected_by;
		// END GEN INTROSPECTOR
	};
}
