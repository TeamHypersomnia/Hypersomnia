#pragma once
#include "augs/misc/enum_associative_array.h"

#include "game/assets/physical_material_id.h"
#include "game/assets/sound_buffer_id.h"

struct physical_material {
	using collision_sound_matrix_type = asset_map<
		assets::physical_material_id, 
		assets::sound_buffer_id
	>;

	// GEN INTROSPECTOR struct physical_material
	collision_sound_matrix_type collision_sound_matrix;
	// END GEN INTROSPECTOR
};