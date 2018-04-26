#include "test_scenes/test_scenes_content.h"
#include "test_scenes/test_scene_sound_buffers.h"

#include "game/assets/physical_material.h"
#include "game/assets/all_logical_assets.h"

void load_test_scene_physical_materials(physical_materials_pool& assets) {
	const auto set_pair = [&](
		const assets::physical_material_id a,
		const assets::physical_material_id b,
		const assets::sound_buffer_id c
	) {
		assets[a].collision_sound_matrix[b] = c;
		assets[b].collision_sound_matrix[a] = c;
	};

	set_pair(assets::physical_material_id::METAL, assets::physical_material_id::METAL, to_sound_id(test_scene_sound_id::COLLISION_METAL_METAL));
	set_pair(assets::physical_material_id::METAL, assets::physical_material_id::WOOD, to_sound_id(test_scene_sound_id::COLLISION_METAL_WOOD));
	set_pair(assets::physical_material_id::WOOD, assets::physical_material_id::WOOD, to_sound_id(test_scene_sound_id::COLLISION_METAL_WOOD));

	set_pair(assets::physical_material_id::GRENADE, assets::physical_material_id::WOOD, to_sound_id(test_scene_sound_id::COLLISION_GRENADE));
	set_pair(assets::physical_material_id::GRENADE, assets::physical_material_id::METAL, to_sound_id(test_scene_sound_id::COLLISION_GRENADE));
	set_pair(assets::physical_material_id::GRENADE, assets::physical_material_id::GRENADE, to_sound_id(test_scene_sound_id::COLLISION_GRENADE));
}