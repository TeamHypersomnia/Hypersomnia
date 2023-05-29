#pragma once
#include "augs/math/declare_math.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "augs/misc/timing/stepped_timing.h"
#include "game/detail/view_input/continuous_rings_input.h"

namespace components {
	struct portal {
		// GEN INTROSPECTOR struct components::portal
		float enter_time_ms = 1000.0f;
		float travel_time_ms = 1000.0f;

		b2Filter custom_filter;
		bool preserve_entry_offset = false;
		pad_bytes<1> pad;

		sentience_shake enter_shake;

		sound_effect_input begin_entering_sound;
		sound_effect_input enter_sound;

		particle_effect_input begin_entering_particles;
		particle_effect_input enter_particles;

		signi_entity_id portal_exit;

		float exit_cooldown_ms = 100.0f;

		sentience_shake exit_shake;
		sound_effect_input exit_sound;
		particle_effect_input exit_particles;

		portal_exit_impulses exit_impulses;

		augs::maybe<continuous_rings_input> rings_effect;

		float highlight_size_mult = 1.0f;
		rgba highlight_color = white;
		// END GEN INTROSPECTOR

		portal() {
			/* Mark component as disabled by default */
			custom_filter.maskBits = 0;
		}
	};
}
