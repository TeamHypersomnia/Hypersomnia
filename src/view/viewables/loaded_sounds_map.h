#pragma once
#include <unordered_map>
#include "game/assets/ids/asset_ids.h"
#include "augs/audio/sound_buffer.h"

using loaded_sounds_map = std::unordered_map<
	assets::sound_id,
	augs::sound_buffer
>;