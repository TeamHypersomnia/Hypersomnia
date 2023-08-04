#pragma once
#include "augs/math/declare_math.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "augs/misc/timing/stepped_timing.h"
#include "game/detail/view_input/continuous_rings_input.h"
#include "game/enums/portal_enums.h"
#include "game/enums/faction_type.h"

namespace components {
	struct portal {
		// GEN INTROSPECTOR struct components::portal
		float enter_time_ms = 1000.0f;
		float travel_time_ms = 1000.0f;

		b2Filter custom_filter;
		rgba_channel decrease_opacity_to = 0;
		bool ignore_airborne_characters = false;

		float begin_entering_highlight_ms = 1000.0f;
		float exit_highlight_ms = 220.0f;

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
		augs::maybe<force_field_def> force_field;
		augs::maybe<hazard_def> hazard;

		portal_exit_direction exit_direction = portal_exit_direction::PORTAL_DIRECTION;
		portal_exit_position exit_position = portal_exit_position::PORTAL_CENTER;

		float light_size_mult = 1.0f;
		rgba light_color = white;
		per_actual_faction<bool> reacts_to_factions = { true, true, true };
		bool ignore_walking_characters = false;
		// END GEN INTROSPECTOR

		portal() {
			/* Mark component as disabled by default */
			custom_filter.maskBits = 0;
		}
	};
}
