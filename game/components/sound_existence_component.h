#pragma once
#include "augs/misc/stepped_timing.h"
#include "game/assets/sound_buffer_id.h"

#include "augs/padding_byte.h"
#include "game/transcendental/entity_handle_declaration.h"

namespace components {
	struct sound_existence {
		struct effect_input {
			assets::sound_buffer_id effect = assets::sound_buffer_id::INVALID;
			bool delete_entity_after_effect_lifetime = true;
			padding_byte pad;
		} input;

		static void activate(const entity_handle);
		static void deactivate(const entity_handle);

		augs::stepped_timestamp time_of_birth;
		unsigned max_lifetime_in_steps = 0u;
	};
}