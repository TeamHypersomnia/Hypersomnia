#include "all.h"

void set_standard_collision_sound_matrix(collision_sound_matrix_type& matrix) {
	const auto set_pair = [&](
		const auto a,
		const auto b,
		const auto c
		) {
		matrix[a][b] = c;
		matrix[b][a] = c;
	};

	set_pair(physical_material_type::METAL, physical_material_type::METAL, assets::sound_buffer_id::COLLISION_METAL_METAL);
	set_pair(physical_material_type::METAL, physical_material_type::WOOD, assets::sound_buffer_id::COLLISION_METAL_WOOD);
	set_pair(physical_material_type::WOOD, physical_material_type::WOOD, assets::sound_buffer_id::COLLISION_METAL_WOOD);
}