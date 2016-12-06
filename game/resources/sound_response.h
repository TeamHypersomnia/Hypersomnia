#pragma once
#include <unordered_map>
#include "game/assets/sound_buffer_id.h"
#include "game/enums/sound_response_type.h"

namespace resources {
	typedef std::unordered_map<sound_response_type, assets::sound_buffer_id> sound_response;
}
