#pragma once
#include "test_scenes/test_scene_physical_materials.h"

real32 get_material_penetrability(test_scene_physical_material_id id);

void create_materials(const intercosm& scene, editor_resource_pools& pools) {
	// auto& materials = scene.world.get_common_significant().logical_assets.physical_materials;
	(void)scene;

	auto& pool = pools.template get_pool_for<editor_material_resource>();

	{
		using test_id_type = test_scene_physical_material_id;

		augs::for_each_enum_except_bounds([&](const test_id_type enum_id) {
			const auto material_id = to_physical_material_id(enum_id);
			// const auto material = materials.get(material_id);

			auto res = editor_material_resource();
			res.unique_name = to_lowercase(augs::enum_to_string(enum_id));
			res.official_tag = enum_id;
			res.scene_asset_id = material_id;

			if (enum_id == test_scene_physical_material_id::GLASS) {
				res.editable.max_ricochet_angle = 40.0f;
			}
			else if (enum_id == test_scene_physical_material_id::WOOD) {
				res.editable.max_ricochet_angle = 10.0f;
			}
			else if (enum_id == test_scene_physical_material_id::METAL) {
				res.editable.max_ricochet_angle = 20.0f;
			}
			else if (enum_id == test_scene_physical_material_id::VENT) {
				res.editable.max_ricochet_angle = 20.0f;
			}

			res.editable.penetrability = get_material_penetrability(enum_id);

			pool.allocate(res);
		});
	}}
