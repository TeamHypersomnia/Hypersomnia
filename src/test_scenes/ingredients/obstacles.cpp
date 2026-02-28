#include "test_scenes/ingredients/ingredients.h"
#include "game/assets/all_logical_assets.h"
#include "game/cosmos/cosmos.h"
#include "game/components/fixtures_component.h"

real32 get_material_penetrability(const test_scene_physical_material_id id) {
	using T = test_scene_physical_material_id;

	switch (id) {
		case T::WOOD:
			return 1.5f;
		case T::METAL:
			return 1.0f;
		case T::GLASS:
			return 2.0f;
		case T::VENT:
			return 1.25f;
		default:
			return 1.0f;
	}
}

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
		) -> auto& {
			meta.template get<invariants::sprite>().tile_excess_size = false;

			test_flavours::add_standard_static_body(meta);

			auto& fixtures_def = meta.template get<invariants::fixtures>();

			fixtures_def.restitution = restitution;
			fixtures_def.density = 100.f;
			fixtures_def.max_ricochet_angle = max_ricochet_angle;
			fixtures_def.material = to_physical_material_id(material);
			fixtures_def.penetrability = get_material_penetrability(material);

			meta.template get<invariants::render>().special_functions.set(special_render_function::ILLUMINATE_AS_WALL);
			meta.template get<invariants::render>().special_functions.set(special_render_function::COVER_GROUND_NEONS);

			return meta;
		};

		const auto glass_alpha = 60;
		const auto glass_neon_alpha = 130;

		auto make_glass_properties = [&](auto& meta) {
			auto& fixtures_def = meta.template get<invariants::fixtures>();
			fixtures_def.filter = filters[predefined_filter_type::GLASS_OBSTACLE];

			meta.template get<invariants::sprite>().color.a = glass_alpha;
			meta.template get<invariants::sprite>().neon_color.a = glass_neon_alpha;
			meta.template get<invariants::render>().special_functions.set(special_render_function::ILLUMINATE_AS_WALL, false);
			meta.template get<invariants::render>().special_functions.set(special_render_function::COVER_GROUND_NEONS, false);

			meta.template get<components::sorting_order>().order = static_cast<sorting_order_type>(test_obstacle_order::GLASS);
		};

		auto static_glass_obstacle = [&](auto& meta) -> auto& {
			static_obstacle(
				meta,
				test_scene_physical_material_id::GLASS,
				0.5f,
				40.f
			);

			make_glass_properties(meta);

			return meta;
		};

		auto dynamic_obstacle = [&](
			auto& meta,
			const auto material
		) -> auto& {
			test_flavours::add_standard_dynamic_body(meta);

			auto& fixtures_def = meta.template get<invariants::fixtures>();

			fixtures_def.filter.maskBits &= ~(1 << int(filter_category::FLYING_EXPLOSIVE));
			fixtures_def.restitution = 0.8f;
			fixtures_def.density = 0.7f;
			fixtures_def.material = to_physical_material_id(material);
			fixtures_def.penetrability = get_material_penetrability(material);

			meta.template get<invariants::render>().special_functions.set(special_render_function::ILLUMINATE_AS_WALL);
			meta.template get<invariants::render>().special_functions.set(special_render_function::COVER_GROUND_NEONS);

			return meta;
		};

		dynamic_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodies::CRATE,
				test_scene_image_id::CRATE,
				test_obstacle_order::OPAQUE
			),
			test_scene_physical_material_id::WOOD
		).template get<invariants::fixtures>().penetrability *= 1.2f;

		static_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodies::HARD_WOODEN_WALL,
				test_scene_image_id::BRICK_WALL,
				test_obstacle_order::OPAQUE
			),
			test_scene_physical_material_id::WOOD
		).template get<invariants::sprite>().tile_excess_size = true;

		{
			auto make_dev_wall = [&](const auto fid, const auto iid) {
				static_obstacle(
					flavour_with_sprite(
						fid,
						iid,
						test_obstacle_order::OPAQUE,
						orange
					),
					test_scene_physical_material_id::METAL,
					0.2f,
					20.f
				).template get<invariants::sprite>().tile_excess_size = true;
			};

			make_dev_wall(test_plain_sprited_bodies::DEV_WALL_32, test_scene_image_id::DEV_FLOOR_32);
			make_dev_wall(test_plain_sprited_bodies::DEV_WALL_64, test_scene_image_id::DEV_FLOOR_64);
			make_dev_wall(test_plain_sprited_bodies::DEV_WALL_128, test_scene_image_id::DEV_FLOOR_128);
			make_dev_wall(test_plain_sprited_bodies::DEV_WALL_256, test_scene_image_id::DEV_FLOOR_256);
		}

		static_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodies::SNACKBAR,
				test_scene_image_id::SNACKBAR,
				test_obstacle_order::OPAQUE
			),

			test_scene_physical_material_id::VENT,
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
		).template get<invariants::render>().layer = render_layer::FOREGROUND;

		static_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodies::LAB_WALL_CORNER_CUT,
				test_scene_image_id::LAB_WALL_CORNER_CUT,
				test_obstacle_order::OPAQUE
			),
			test_scene_physical_material_id::METAL,
			0.2f,
			20.f
		).template get<invariants::render>().layer = render_layer::FOREGROUND;

		static_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodies::LAB_WALL_CORNER_SQUARE,
				test_scene_image_id::LAB_WALL_CORNER_SQUARE,
				test_obstacle_order::OPAQUE
			),
			test_scene_physical_material_id::METAL,
			0.2f,
			20.f
		).template get<invariants::render>().layer = render_layer::FOREGROUND;

		static_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodies::LAB_WALL,
				test_scene_image_id::LAB_WALL,
				test_obstacle_order::OPAQUE
			),
			test_scene_physical_material_id::METAL,
			0.2f,
			20.f
		).template get<invariants::render>().layer = render_layer::FOREGROUND;

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

		{

			const auto default_size = vec2i(128, 128);

			auto create_default_collider = [&](
				const auto id,
				const auto triangle_id,
				const auto material,
				const auto color,
				const auto restitution,
				const auto max_ricochet_angle,
				const bool glass = false
			) {
				{
					auto& meta = flavour_with_sprite(
						id,
						test_scene_image_id::BLANK,
						test_obstacle_order::OPAQUE,
						color
					);

					static_obstacle(
						meta,
						material,
						restitution,
						max_ricochet_angle
					);

					meta.template get<invariants::sprite>().size = default_size;

					if (glass) {
						make_glass_properties(meta);
						meta.template get<invariants::sprite>().color.a = 200;
					}
				}

				{
					auto& meta = flavour_with_sprite(
						triangle_id,
						test_scene_image_id::TRIANGLE_COLLIDER,
						test_obstacle_order::OPAQUE,
						color
					);

					static_obstacle(
						meta,
						material,
						restitution,
						max_ricochet_angle
					);

					meta.template get<invariants::sprite>().size = default_size;

					if (glass) {
						make_glass_properties(meta);
						meta.template get<invariants::sprite>().color.a = 200;
					}
				}
			};

			create_default_collider(
				test_plain_sprited_bodies::BOX_COLLIDER_WOOD,
				test_plain_sprited_bodies::TRIANGLE_COLLIDER_WOOD,
				test_scene_physical_material_id::WOOD,
				rgba(255, 150, 50, 255),
				0.0f,
				10.0f
			);

			create_default_collider(
				test_plain_sprited_bodies::BOX_COLLIDER_METAL,
				test_plain_sprited_bodies::TRIANGLE_COLLIDER_METAL,
				test_scene_physical_material_id::METAL,
				rgba(200, 200, 255, 255),
				0.2f,
				20.0f
			);

			create_default_collider(
				test_plain_sprited_bodies::BOX_COLLIDER_VENT,
				test_plain_sprited_bodies::TRIANGLE_COLLIDER_VENT,
				test_scene_physical_material_id::VENT,
				rgba(200, 200, 200, 255),
				0.2f,
				20.0f
			);

			create_default_collider(
				test_plain_sprited_bodies::BOX_COLLIDER_GLASS,
				test_plain_sprited_bodies::TRIANGLE_COLLIDER_GLASS,
				test_scene_physical_material_id::GLASS,
				rgba(200, 255, 255, 255),
				0.5f,
				40.0f,
				true
			);
		}
	}
}
