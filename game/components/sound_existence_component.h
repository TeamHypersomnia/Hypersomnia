#pragma once
#include "augs/misc/stepped_timing.h"
#include "game/assets/sound_buffer_id.h"

#include "augs/padding_byte.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "augs/audio/sound_effect_modifier.h"

struct sound_effect_input {
	// GEN INTROSPECTOR struct sound_effect_input
	assets::sound_buffer_id effect = assets::sound_buffer_id::INVALID;
	bool delete_entity_after_effect_lifetime = true;
	char variation_number = -1;
	entity_id direct_listener;
	augs::sound_effect_modifier modifier;
	// END GEN INTROSPECTOR
};

namespace components {
	struct sound_existence {
		// GEN INTROSPECTOR struct components::sound_existence
		sound_effect_input input;
		// END GEN INTROSPECTOR

		static bool is_activated(const const_entity_handle);
		static void activate(const entity_handle);
		static void deactivate(const entity_handle);

		augs::stepped_timestamp time_of_birth;
		unsigned max_lifetime_in_steps = 0u;

		float calculate_max_audible_distance() const;

		size_t random_variation_number_from_transform(const components::transform) const;
	};
}