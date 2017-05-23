#include "generated/setting_build_test_scenes.h"
#if BUILD_TEST_SCENES
#include "all.h"

#include "game/assets/physical_material.h"
#include "game/assets/assets_manager.h"

void set_standard_physical_materials(assets_manager& assets) {
	const auto set_pair = [&](
		const assets::physical_material_id a,
		const assets::physical_material_id b,
		const assets::sound_buffer_id c
	) {
		assets[a].collision_sound_matrix[b] = c;
		assets[b].collision_sound_matrix[a] = c;
	};

	set_pair(assets::physical_material_id::METAL, assets::physical_material_id::METAL, assets::sound_buffer_id::COLLISION_METAL_METAL);
	set_pair(assets::physical_material_id::METAL, assets::physical_material_id::WOOD, assets::sound_buffer_id::COLLISION_METAL_WOOD);
	set_pair(assets::physical_material_id::WOOD, assets::physical_material_id::WOOD, assets::sound_buffer_id::COLLISION_METAL_WOOD);

	set_pair(assets::physical_material_id::GRENADE, assets::physical_material_id::WOOD, assets::sound_buffer_id::COLLISION_GRENADE);
	set_pair(assets::physical_material_id::GRENADE, assets::physical_material_id::METAL, assets::sound_buffer_id::COLLISION_GRENADE);
	set_pair(assets::physical_material_id::GRENADE, assets::physical_material_id::GRENADE, assets::sound_buffer_id::COLLISION_GRENADE);
}
#endif