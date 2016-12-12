#pragma once
#include <unordered_map>
#include "game/assets/sound_buffer_id.h"
#include "game/enums/sound_response_type.h"
#include "augs/audio/sound_effect_modifier.h"

namespace resources {
	struct sound_response_entry {
		assets::sound_buffer_id id;
		augs::sound_effect_modifier modifier;

		sound_response_entry(
			const assets::sound_buffer_id id = assets::sound_buffer_id::INVALID,
			augs::sound_effect_modifier modifier = augs::sound_effect_modifier()
		) : id(id), modifier(modifier) {}
	};

	typedef std::unordered_map<sound_response_type, sound_response_entry> sound_response;
}
