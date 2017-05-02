#pragma once
#include "augs/misc/enum_associative_array.h"

#include "game/assets/physical_material_id.h"
#include "game/assets/sound_buffer_id.h"

class assets_manager;

struct physical_material {
	typedef augs::enum_associative_array<
		assets::physical_material_id, 
		assets::sound_buffer_id
	> collision_sound_matrix_type;

	// GEN INTROSPECTOR struct physical_material
	collision_sound_matrix_type collision_sound_matrix;
	// END GEN INTROSPECTOR

	physical_material get_logical_meta(const assets_manager& manager) const {
		return *this;
	}
};