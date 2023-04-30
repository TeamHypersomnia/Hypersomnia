#pragma once
#include "test_scenes/test_scene_physical_materials.h"

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
			else {
				/* 
					For now only allow well-defined wall resources.
					Skip grenade, flashbang, knife etc.
				*/

				return;
			}

			pool.allocate(res);
		});
	}
#if CREATE_OFFICIAL_CONTENT_ON_EDITOR_LEVEL
	auto create_material = [&](const official_materials id) -> auto& {
		return create_official(id, pools).editable;
	};

	{
		auto& glass = create_material(official_materials::GLASS);

		glass.standard_damage_sound.resource_id = to_resource_id(official_sounds::GLASS_DAMAGE);

		glass.standard_damage_particles.resource_id = to_resource_id(official_particles::GLASS_DAMAGE);
		glass.standard_damage_particles.colorize = rgba(142, 186, 197, 255);
	}

	//using bound = augs::bound<real32>;

	const auto set_pair = [&](
		const official_materials a,
		const official_materials b,
		const official_sounds c,
		const bool both_ways = true,
		std::optional<editor_collision_sound_def> maybe_def_template = std::nullopt
	) {
		const auto a_id = to_resource_id(a);
		const auto b_id = to_resource_id(b);
		const auto c_id = to_resource_id(c);

		auto def_template = editor_collision_sound_def();

		if (maybe_def_template.has_value()) {
			def_template = *maybe_def_template;
		}
		else {
			def_template.occurences_before_cooldown = 3;
		}

		{
			auto& entry = get_resource(a_id, pools).collision_sound_matrix[b_id];
			def_template.effect.resource_id = c_id;
			entry = def_template;
		}

		if (both_ways) {
			auto& entry = get_resource(b_id, pools).collision_sound_matrix[a_id];

			def_template.effect.resource_id = c_id;
			entry = def_template;
		}
	};

	set_pair(official_materials::GLASS, official_materials::GLASS, official_sounds::COLLISION_GLASS);
#endif
}
