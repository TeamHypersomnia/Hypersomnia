#pragma once
#include "augs/math/declare_math.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "augs/misc/timing/stepped_timing.h"

namespace components {
	struct portal {
		// GEN INTROSPECTOR struct components::portal
		float enter_time_ms = 1000.0f;
		float travel_time_ms = 1000.0f;

		sentience_shake enter_shake;
		sentience_shake exit_shake;

		b2Filter custom_filter;
		bool exit_preserves_entry_offset = false;
		pad_bytes<1> pad;

		sound_effect_input begin_entering_sound;
		sound_effect_input enter_sound;
		sound_effect_input exit_sound;

		particle_effect_input begin_entering_particles;
		particle_effect_input enter_particles;
		particle_effect_input exit_particles;

		signi_entity_id portal_exit;
		// END GEN INTROSPECTOR

		portal() {
			/* Mark component as disabled by default */
			custom_filter.maskBits = 0;
		}
	};
}
