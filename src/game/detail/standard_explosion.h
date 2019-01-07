#pragma once
#include "augs/pad_bytes.h"

#include "augs/graphics/rgba.h"
#include "augs/misc/value_meter.h"

#include "game/assets/ids/asset_ids.h"
#include "game/components/transform_component.h"
#include "game/cosmos/step_declaration.h"
#include "game/cosmos/entity_id_declaration.h"
#include "game/enums/adverse_element_type.h"
#include "game/detail/sentience_shake.h"
#include "game/detail/damage/damage_definition.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/predictability_info.h"

struct damage_cause;

struct standard_explosion_input {
	// GEN INTROSPECTOR struct standard_explosion_input
	real32 effective_radius = 250.f;

	damage_definition damage;

	sentience_shake subject_shake;

	real32 subject_impulse = 0.f;
	real32 subject_inert_ms = 0.f;

	real32 wave_shake_radius_mult = 2.f;
	rgba inner_ring_color = cyan;
	rgba outer_ring_color = white;
	sound_effect_input sound;
	real32 ring_duration_seconds = 0.20f;
	adverse_element_type type = adverse_element_type::FORCE;
	bool create_thunders_effect = false;
	pad_bytes<3> pad;
	// END GEN INTROSPECTOR

	auto& operator*=(const real32 scalar) {
		damage *= scalar;
		effective_radius *= scalar;
		subject_shake *= scalar;
		subject_impulse *= scalar;
		subject_inert_ms *= scalar;

		return *this;
	}

	void instantiate(
		logic_step step, 
		transformr explosion_location, 
		damage_cause cause,
		predictability_info info = always_predictable_v
	) const;
};
