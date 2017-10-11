#pragma once
#include "augs/misc/timing/stepped_timing.h"
#include "game/assets/ids/sound_buffer_id.h"

#include "augs/pad_bytes.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/step_declaration.h"

#include "game/detail/view_input/sound_effect_input.h"

struct sound_existence_input {
	// GEN INTROSPECTOR struct sound_existence_input
	sound_effect_input effect;
	bool delete_entity_after_effect_lifetime = true;
	char variation_number = -1;
	pad_bytes<2> pad;
	entity_id direct_listener;
	// END GEN INTROSPECTOR

	entity_handle create_sound_effect_entity(
		const logic_step step,
		const components::transform place_of_birth,
		const entity_id chased_subject_id
	) const;
};

namespace components {
	struct sound_existence {
		// GEN INTROSPECTOR struct components::sound_existence
		sound_existence_input input;

		augs::stepped_timestamp time_of_birth;
		unsigned max_lifetime_in_steps = 0u;
		// END GEN INTROSPECTOR

		static bool is_activated(const const_entity_handle);
		static void activate(const entity_handle);
		static void deactivate(const entity_handle);

		float calculate_max_audible_distance() const;

		size_t random_variation_number_from_transform(const components::transform) const;
	};
}