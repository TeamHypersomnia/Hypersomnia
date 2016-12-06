#pragma once
#include "augs/misc/stepped_timing.h"
#include "game/assets/sound_buffer_id.h"

namespace components {
	struct sound_existence {
		struct effect_input {
			assets::sound_buffer_id effect = assets::sound_buffer_id::INVALID;
		} input;

		augs::stepped_timestamp time_of_birth;
		unsigned max_lifetime_in_steps = 0u;
	};
}