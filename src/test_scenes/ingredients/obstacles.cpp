#include "ingredients.h"
#include "game/assets/all_logical_assets.h"
#include "game/transcendental/cosmos.h"
#include "game/components/type_component.h"
#include "game/components/fixtures_component.h"

namespace test_types {
	void populate_crate_types(const loaded_game_image_caches& logicals, entity_types& types) {
		{
			auto& meta = get_test_type(types, test_scene_type::CRATE);

			invariants::render render_def;
			render_def.layer = render_layer::DYNAMIC_BODY;

			meta.set(render_def);

			test_types::add_sprite(meta, logicals, assets::game_image_id::CRATE, white);
			meta.add_shape_invariant_from_renderable(logicals);

			test_types::add_standard_dynamic_body(meta);

			auto& fixtures_def = meta.get<invariants::fixtures>();

			fixtures_def.restitution = 0.8f;
			fixtures_def.density = 0.7f;
			fixtures_def.material = assets::physical_material_id::WOOD;
		}

		{
			auto& meta = get_test_type(types, test_scene_type::BRICK_WALL);

			invariants::render render_def;
			render_def.layer = render_layer::DYNAMIC_BODY;

			meta.set(render_def);

			test_types::add_sprite(meta, logicals, assets::game_image_id::BRICK_WALL, white);
			meta.get<invariants::sprite>().size = vec2(160, 160);
			meta.add_shape_invariant_from_renderable(logicals);

			test_types::add_standard_static_body(meta);

			auto& fixtures_def = meta.get<invariants::fixtures>();

			fixtures_def.restitution = 0.0f;
			fixtures_def.density = 100.f;
			fixtures_def.material = assets::physical_material_id::WOOD;
		}
	}
}

namespace prefabs {
	entity_handle create_crate(const logic_step step, const components::transform pos) {
		const auto crate = create_test_scene_entity(step.get_cosmos(), test_scene_type::CRATE);
		crate.set_logic_transform(pos);
		crate.add_standard_components(step);
		return crate;
	}

	entity_handle create_brick_wall(const logic_step step, const components::transform pos) {
		const auto crate = create_test_scene_entity(step.get_cosmos(), test_scene_type::BRICK_WALL);
		crate.set_logic_transform(pos);
		crate.add_standard_components(step);
		return crate;
	}
}
