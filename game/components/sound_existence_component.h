#pragma once
#include "augs/misc/stepped_timing.h"
#include "game/assets/sound_buffer_id.h"

#include "augs/padding_byte.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "augs/audio/sound_effect_modifier.h"

namespace components {
	struct sound_existence {
		struct effect_input {
			assets::sound_buffer_id effect = assets::sound_buffer_id::INVALID;
			bool delete_entity_after_effect_lifetime = true;
			char variation_number = -1;
			entity_id direct_listener;
			augs::sound_effect_modifier modifier;
		} input;

		template<class F>
		void for_each_held_id(F f) {
			f(input.direct_listener);
		}

		template<class F>
		void for_each_held_id(F f) const {
			f(input.direct_listener);
		}

		static bool is_activated(const const_entity_handle);
		static void activate(const entity_handle);
		static void deactivate(const entity_handle);

		augs::stepped_timestamp time_of_birth;
		unsigned max_lifetime_in_steps = 0u;

		float calculate_max_audible_distance() const;

		size_t random_variation_number_from_transform(const components::transform) const;
	};
}