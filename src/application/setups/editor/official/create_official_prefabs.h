#pragma once
#include "test_scenes/test_scene_flavour_ids.h"
#include "test_scenes/test_scene_flavours.h"

void editor_setup::create_official_prefabs() {
	using id_type = editor_builtin_prefab_type;

	auto& pool = official_resources.template get_pool_for<editor_prefab_resource>();

	augs::for_each_enum_except_bounds([&](const id_type enum_id) {
		auto res = editor_prefab_resource();
		res.unique_name = to_lowercase(augs::enum_to_string(enum_id));
		res.editable.type = enum_id;

		using T = editor_builtin_prefab_type;

		auto& defs = res.editable.default_node_properties;

		auto& m = official_resource_map;

		switch (enum_id) {
			case T::AQUARIUM:
				{
					defs.size = { 384 * 2, 384 * 2 };

					auto& a = defs.as_aquarium;

					a.sand_1 = m[test_static_decorations::AQUARIUM_SAND_1];
					a.sand_2 = m[test_static_decorations::AQUARIUM_SAND_2];
					a.sand_edge = m[test_static_decorations::AQUARIUM_SAND_EDGE];

					a.dune_small = m[test_static_decorations::DUNE_SMALL];
					a.dune_big = m[test_static_decorations::DUNE_BIG];

					a.wall = m[test_plain_sprited_bodies::LAB_WALL];
					a.wall_top_corners = m[test_plain_sprited_bodies::LAB_WALL_CORNER_CUT];
					a.wall_bottom_corners = m[test_plain_sprited_bodies::LAB_WALL_CORNER_CUT];
					a.wall_smooth_end = m[test_plain_sprited_bodies::LAB_WALL_SMOOTH_END];

					a.wall_top_foreground = m[test_static_decorations::LAB_WALL_A2_FOREGROUND];

					a.glass = m[test_plain_sprited_bodies::AQUARIUM_GLASS];
					a.glass_start = m[test_plain_sprited_bodies::AQUARIUM_GLASS_START];

					a.sand_lamp_body =  m[test_static_decorations::AQUARIUM_BOTTOM_LAMP_BODY];
					a.sand_lamp_light = m[test_static_decorations::AQUARIUM_BOTTOM_LAMP_LIGHT];

					a.wall_lamp_body =  m[test_static_decorations::AQUARIUM_HALOGEN_1_BODY];
					a.wall_lamp_light = m[test_static_decorations::AQUARIUM_HALOGEN_1_LIGHT];

					a.flower_1 = m[test_dynamic_decorations::FLOWER_PINK];
					a.flower_2 = m[test_dynamic_decorations::FLOWER_CYAN];
					a.coral = m[test_dynamic_decorations::PINK_CORAL];

					a.fish_1 = m[test_dynamic_decorations::YELLOW_FISH];
					a.fish_2 = m[test_dynamic_decorations::DARKBLUE_FISH];
					a.fish_3 = m[test_dynamic_decorations::CYANVIOLET_FISH];
					a.fish_4 = m[test_dynamic_decorations::JELLYFISH];
					a.fish_5 = m[test_dynamic_decorations::DRAGON_FISH];
					a.fish_6 = m[test_dynamic_decorations::RAINBOW_DRAGON_FISH];

					a.bubbles = m[test_particles_decorations::AQUARIUM_BUBBLES];
					a.flower_bubbles = m[test_particles_decorations::FLOWER_BUBBLES];

					a.wandering_pixels_1 = m[test_wandering_pixels_decorations::AQUARIUM_PIXELS_DIM];
					a.wandering_pixels_2 = m[test_wandering_pixels_decorations::AQUARIUM_PIXELS_LIGHT];

					a.ambience_left =  m[test_sound_decorations::AQUARIUM_AMBIENCE_LEFT];
					a.ambience_right = m[test_sound_decorations::AQUARIUM_AMBIENCE_RIGHT];

					a.water_overlay = m[test_static_decorations::WATER_COLOR_OVERLAY];
					//a.collider_interior = m[test_plain_sprited_bodies::BOX_COLLIDER_GLASS];

					a.caustics = m[test_dynamic_decorations::WATER_CAUSTICS];
				}
				break;

			default:
				break;
		}

		pool.allocate(res);
	});
}
