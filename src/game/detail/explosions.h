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

struct standard_explosion_input {
	// GEN INTROSPECTOR struct standard_explosion_input
	float effective_radius = 250.f;
	meter_value_type damage = 88;
	float impact_impulse = 150.f;

	sentience_shake victim_shake;
	sentience_shake subject_shake;

	rgba inner_ring_color = cyan;
	rgba outer_ring_color = white;
	assets::sound_id sound_effect;
	float sound_gain = 1.f;
	adverse_element_type type = adverse_element_type::FORCE;
	bool create_thunders_effect = false;
	pad_bytes<3> pad;
	// END GEN INTROSPECTOR

	auto& operator*=(const real32 scalar) {
		effective_radius *= scalar;
		damage *= scalar;
		impact_impulse *= scalar;

		victim_shake *= scalar;
		subject_shake *= scalar;
		return *this;
	}

	void instantiate(
		logic_step step, 
		transformr explosion_location, 
		entity_id subject_if_any
	) const;

	void instantiate_no_subject(
		logic_step step, 
		transformr explosion_location
	) const;
};
