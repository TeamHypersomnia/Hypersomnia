#include "augs/templates/enum_introspect.h"
#include "test_scenes/test_scene_physical_materials.h"
#include "test_scenes/test_scenes_content.h"
#include "test_scenes/test_scene_sounds.h"

#include "game/assets/physical_material.h"
#include "game/assets/all_logical_assets.h"

void load_test_scene_physical_materials(physical_materials_pool& all_definitions) {
	using test_id_type = test_scene_physical_material_id;

	all_definitions.reserve(enum_count(test_id_type()));

	augs::for_each_enum_except_bounds([&](const test_id_type) {
		all_definitions.allocate();
	});

	const auto set_pair = [&](
		const assets::physical_material_id a,
		const assets::physical_material_id b,
		const assets::sound_id c
	) {
		all_definitions[a].collision_sound_matrix[b] = c;
		all_definitions[b].collision_sound_matrix[a] = c;
	};

	set_pair(to_physical_material_id(test_scene_physical_material_id::METAL), to_physical_material_id(test_scene_physical_material_id::METAL), to_sound_id(test_scene_sound_id::COLLISION_METAL_METAL));
	set_pair(to_physical_material_id(test_scene_physical_material_id::METAL), to_physical_material_id(test_scene_physical_material_id::WOOD), to_sound_id(test_scene_sound_id::COLLISION_METAL_WOOD));
	set_pair(to_physical_material_id(test_scene_physical_material_id::WOOD), to_physical_material_id(test_scene_physical_material_id::WOOD), to_sound_id(test_scene_sound_id::COLLISION_METAL_WOOD));

	set_pair(to_physical_material_id(test_scene_physical_material_id::GRENADE), to_physical_material_id(test_scene_physical_material_id::WOOD), to_sound_id(test_scene_sound_id::COLLISION_GRENADE));
	set_pair(to_physical_material_id(test_scene_physical_material_id::GRENADE), to_physical_material_id(test_scene_physical_material_id::METAL), to_sound_id(test_scene_sound_id::COLLISION_GRENADE));
	set_pair(to_physical_material_id(test_scene_physical_material_id::GRENADE), to_physical_material_id(test_scene_physical_material_id::GRENADE), to_sound_id(test_scene_sound_id::COLLISION_GRENADE));
}