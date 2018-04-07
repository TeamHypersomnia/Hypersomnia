#pragma once
#include "augs/misc/enum/enum_map.h"

#include "view/viewables/all_viewables_declarations.h"

#include "game/assets/ids/sound_buffer_id.h"
#include "augs/audio/sound_buffer.h"

struct loaded_sounds_map : public asset_map<
	assets::sound_buffer_id,
	augs::sound_buffer
> {
	loaded_sounds_map() = default;
	explicit loaded_sounds_map(const sound_buffer_inputs_map&);
};