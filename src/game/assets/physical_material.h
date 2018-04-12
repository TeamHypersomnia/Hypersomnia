#pragma once
#include "augs/misc/enum/enum_map.h"

#include "game/assets/ids/physical_material_id.h"
#include "game/assets/ids/sound_buffer_id.h"

struct physical_material {
	using collision_sound_matrix_type = augs::enum_map<
		assets::physical_material_id, 
		assets::sound_buffer_id
	>;

	// GEN INTROSPECTOR struct physical_material
	collision_sound_matrix_type collision_sound_matrix;
	std::string name;
	// END GEN INTROSPECTOR
};