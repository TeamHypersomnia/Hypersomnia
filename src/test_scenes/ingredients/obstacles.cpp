#include "test_scenes/ingredients/ingredients.h"
#include "game/assets/all_logical_assets.h"
#include "game/transcendental/cosmos.h"
#include "game/components/fixtures_component.h"

namespace test_flavours {
	void populate_crate_flavours(const populate_flavours_input in) {
		auto& caches = in.caches;
		auto flavour_with_sprite = in.flavour_with_sprite_maker();

		auto static_obstacle = [&](
			auto& meta,
			const auto material,
			const float restitution = 0.f,
			const float max_ricochet_angle = 10.f
		) {
			add_shape_invariant_from_renderable(meta, caches);

			test_flavours::add_standard_static_body(meta);

			auto& fixtures_def = meta.template get<invariants::fixtures>();

			fixtures_def.restitution = restitution;
			fixtures_def.density = 100.f;
			fixtures_def.max_ricochet_angle = max_ricochet_angle;
			fixtures_def.material = to_physical_material_id(material);
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
			fixtures_def.filter = filters::glass_obstacle();

			meta.template get<invariants::sprite>().color.a = glass_alpha;
			meta.template get<invariants::sprite>().neon_color.a = glass_neon_alpha;
		};

		auto dynamic_obstacle = [&](
			auto& meta,
			const auto material
		) {
			add_shape_invariant_from_renderable(meta, caches);

			test_flavours::add_standard_dynamic_body(meta);

			auto& fixtures_def = meta.template get<invariants::fixtures>();

			fixtures_def.restitution = 0.8f;
			fixtures_def.density = 0.7f;
			fixtures_def.material = to_physical_material_id(material);
		};

		dynamic_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodys::CRATE,
				test_scene_image_id::CRATE,
				render_layer::DYNAMIC_BODY
			),
			test_scene_physical_material_id::WOOD
		);

		static_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodys::BRICK_WALL,
				test_scene_image_id::BRICK_WALL,
				render_layer::DYNAMIC_BODY
			),
			test_scene_physical_material_id::WOOD
		);


		static_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodys::LAB_WALL_SMOOTH_END,
				test_scene_image_id::LAB_WALL_SMOOTH_END,
				render_layer::DYNAMIC_BODY
			),
			test_scene_physical_material_id::METAL
		);

		static_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodys::LAB_WALL_CORNER_CUT,
				test_scene_image_id::LAB_WALL_CORNER_CUT,
				render_layer::DYNAMIC_BODY
			),
			test_scene_physical_material_id::METAL
		);

		static_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodys::LAB_WALL_CORNER_SQUARE,
				test_scene_image_id::LAB_WALL_CORNER_SQUARE,
				render_layer::DYNAMIC_BODY
			),
			test_scene_physical_material_id::METAL
		);

		static_glass_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodys::AQUARIUM_GLASS,
				test_scene_image_id::AQUARIUM_GLASS,
				render_layer::GLASS_BODY
			)
		);

		static_glass_obstacle(
			flavour_with_sprite(
				test_plain_sprited_bodys::AQUARIUM_GLASS_START,
				test_scene_image_id::AQUARIUM_GLASS_START,
				render_layer::GLASS_BODY
			)
		);
	}
}

namespace prefabs {
	entity_handle create_crate(const logic_step step, const transformr pos) {
		const auto crate = create_test_scene_entity(step.get_cosmos(), test_plain_sprited_bodys::CRATE, pos);
		return crate;
	}

	entity_handle create_brick_wall(const logic_step step, transformr pos) {
		const auto crate = create_test_scene_entity(step.get_cosmos(), test_plain_sprited_bodys::BRICK_WALL, pos);
		return crate;
	}
}
