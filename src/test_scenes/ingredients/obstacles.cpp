#include "test_scenes/ingredients/ingredients.h"
#include "game/assets/all_logical_assets.h"
#include "game/transcendental/cosmos.h"
#include "game/components/fixtures_component.h"

namespace test_flavours {
	void populate_crate_flavours(const populate_flavours_input in) {
		auto& caches = in.caches;
		auto& flavours = in.flavours;

		{
			auto& meta = get_test_flavour(flavours, test_plain_sprited_bodys::CRATE);

			invariants::render render_def;
			render_def.layer = render_layer::DYNAMIC_BODY;

			meta.set(render_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::CRATE, white);
			add_shape_invariant_from_renderable(meta, caches);

			test_flavours::add_standard_dynamic_body(meta);

			auto& fixtures_def = meta.get<invariants::fixtures>();

			fixtures_def.restitution = 0.8f;
			fixtures_def.density = 0.7f;
			fixtures_def.material = to_physical_material_id(test_scene_physical_material_id::WOOD);
		}

		{
			auto& meta = get_test_flavour(flavours, test_plain_sprited_bodys::BRICK_WALL);

			invariants::render render_def;
			render_def.layer = render_layer::DYNAMIC_BODY;

			meta.set(render_def);

			test_flavours::add_sprite(meta, caches, test_scene_image_id::BRICK_WALL, white);
			meta.get<invariants::sprite>().size = vec2(128, 128);
			add_shape_invariant_from_renderable(meta, caches);

			test_flavours::add_standard_static_body(meta);

			auto& fixtures_def = meta.get<invariants::fixtures>();

			fixtures_def.restitution = 0.0f;
			fixtures_def.density = 100.f;
			fixtures_def.material = to_physical_material_id(test_scene_physical_material_id::WOOD);
		}
	}
}

namespace prefabs {
	entity_handle create_crate(const logic_step step, const components::transform pos) {
		const auto crate = create_test_scene_entity(step.get_cosmos(), test_plain_sprited_bodys::CRATE, pos);
		return crate;
	}

	entity_handle create_brick_wall(const logic_step step, components::transform pos) {
		const auto crate = create_test_scene_entity(step.get_cosmos(), test_plain_sprited_bodys::BRICK_WALL, pos);
		return crate;
	}
}
