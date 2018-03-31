#pragma once
#include "augs/pad_bytes.h"

#include "augs/graphics/rgba.h"
#include "augs/misc/value_meter.h"

#include "game/assets/ids/sound_buffer_id.h"
#include "game/components/transform_component.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_id_declaration.h"
#include "game/enums/adverse_element_type.h"
#include "game/detail/sentience_shake.h"

struct standard_explosion_input {
	// GEN INTROSPECTOR struct standard_explosion_input
	float effective_radius = 250.f;
	meter_value_type damage = 88;
	float impact_impulse = 150.f;

	sentience_shake victim_shake;
	sentience_shake sender_shake;

	rgba inner_ring_color = cyan;
	rgba outer_ring_color = white;
	assets::sound_buffer_id sound_effect = assets::sound_buffer_id::INVALID;
	float sound_gain = 1.f;
	adverse_element_type type = adverse_element_type::INVALID;
	bool create_thunders_effect = false;
	pad_bytes<3> pad;
	// END GEN INTROSPECTOR

	void instantiate(
		const logic_step step, 
		const components::transform explosion_location, 
		const entity_id subject_if_any
	) const;
};
