#include "test_scenes/ingredients/ingredients.h"
#include "game/assets/all_logical_assets.h"
#include "game/cosmos/cosmos.h"
#include "game/components/fixtures_component.h"

namespace test_flavours {
	void populate_crate_flavours(const populate_flavours_input in) {
		auto& caches = in.caches;
		auto flavour_with_sprite = in.flavour_with_sprite_maker();

		(void)caches;

		auto static_obstacle = [&](
			auto& meta,
			const auto material,
			const float restitution = 0.f,
			const float max_ricochet_angle = 10.f
		) {
			meta.template get<invariants::sprite>().tile_excess_size = true;

			test_flavours::add_standard_static_body(meta);

			auto& fixtures_def = meta.template get<invariants::fixtures>();

			fixtures_def.restitution = restitution;
			fixtures_def.density = 100.f;
			fixtures_def.max_ricochet_angle = max_ricochet_angle;
			fixtures_def.material = to_physical_material_id(material);

			meta.template get<invariants::render>().special_functions.set(special_render_function::ILLUMINATE_AS_WALL);
			meta.template get<invariants::render>().special_functions.set(special_render_function::COVER_GROUND_NEONS);
		};

		const auto glass_alpha = 60;
		const auto glass_neon_alpha = 130;

		auto static_glass_obstacle = [&](auto& meta) {
			static_obstacle(
				meta,
				test_scene_physical_material_id::GLASS,
				0.5f,
				40.f
			);

			auto& fixtures_def = meta.template get<invariants::fixtures>();
			fixtures_def.filter = filters[predefined_filter_type::GLASS_OBSTACLE];

			meta.template get<invariants::sprite>().color.a = glass_alpha;
			meta.template get<invariants::sprite>().neon_color.a = glass_neon_alpha;
			meta.template get<invariants::render>().special_functions.set(special_render_function::ILLUMINATE_AS_WALL, false);
			meta.template get<invariants::render>().special_functions.set(special_render_function::COVER_GROUND_NEONS, false);
		};

		auto dynamic_obstacle = [&](
			auto& meta,
			const auto material
		) {
			test_flavours::add_standard_dynamic_body(meta);

			auto& fixtures_def = meta.template get<invariants::fixtures>();

			fixtures_def.restitution = 0.8f;
			fixtures_def.density = 0.7f;
			fixtures_def.material = to_physical_material_id(material);

			meta.template get<invariants::render>().special_functions.set(special_render_function::ILLUMINATE_AS_WALL);
			meta.template get<invariants::render>().special_functions.set(special_render_function::COVER_GROUND_NEONS);
		};

		dynamic_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodies::CRATE,
				test_scene_image_id::CRATE,
				test_obstacle_order::OPAQUE
			),
			test_scene_physical_material_id::WOOD
		);

		static_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodies::HARD_WOODEN_WALL,
				test_scene_image_id::BRICK_WALL,
				test_obstacle_order::OPAQUE
			),
			test_scene_physical_material_id::WOOD
		);

		static_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodies::SNACKBAR,
				test_scene_image_id::SNACKBAR,
				test_obstacle_order::OPAQUE
			),

			test_scene_physical_material_id::AIR_DUCT,
			0.2f,
			20.f
		);

		static_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodies::LAB_WALL_SMOOTH_END,
				test_scene_image_id::LAB_WALL_SMOOTH_END,
				test_obstacle_order::OPAQUE
			),
			test_scene_physical_material_id::METAL,
			0.2f,
			20.f
		);

		static_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodies::LAB_WALL_CORNER_CUT,
				test_scene_image_id::LAB_WALL_CORNER_CUT,
				test_obstacle_order::OPAQUE
			),
			test_scene_physical_material_id::METAL,
			0.2f,
			20.f
		);

		static_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodies::LAB_WALL_CORNER_SQUARE,
				test_scene_image_id::LAB_WALL_CORNER_SQUARE,
				test_obstacle_order::OPAQUE
			),
			test_scene_physical_material_id::METAL,
			0.2f,
			20.f
		);

		static_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodies::LAB_WALL,
				test_scene_image_id::LAB_WALL,
				test_obstacle_order::OPAQUE
			),
			test_scene_physical_material_id::METAL,
			0.2f,
			20.f
		);

		static_glass_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodies::AQUARIUM_GLASS,
				test_scene_image_id::AQUARIUM_GLASS,
				test_obstacle_order::GLASS
			)
		);

		static_glass_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodies::AQUARIUM_GLASS_START,
				test_scene_image_id::AQUARIUM_GLASS_START,
				test_obstacle_order::GLASS
			)
		);
	}
}
