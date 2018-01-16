#include "ingredients.h"
#include "game/assets/all_logical_assets.h"
#include "game/transcendental/cosmos.h"
#include "game/components/type_component.h"
#include "game/components/fixtures_component.h"

namespace test_types {
	void populate_crate_types(const all_logical_assets& logicals, entity_types& types) {
		{
			auto& meta = get_test_type(types, test_scene_type::CRATE);

			definitions::render render_def;
			render_def.layer = render_layer::DYNAMIC_BODY;

			meta.set(render_def);


			test_types::add_sprite(meta, logicals, assets::game_image_id::CRATE, white);
			meta.add_shape_definition_from_renderable(logicals);
		}

		{
			auto& meta = get_test_type(types, test_scene_type::BRICK_WALL);

			definitions::render render_def;
			render_def.layer = render_layer::DYNAMIC_BODY;

			meta.set(render_def);

			test_types::add_sprite(meta, logicals, assets::game_image_id::BRICK_WALL, white);
			meta.add_shape_definition_from_renderable(logicals);
		}
	}
}

namespace prefabs {
	entity_handle create_crate(const logic_step step, const components::transform pos, vec2i size) {
		const auto crate = create_test_scene_entity(step.get_cosmos(), test_scene_type::CRATE);
		
		if (size.is_zero()) {
			size = step.get_logical_assets().at(assets::game_image_id::CRATE).get_size();
		}

		ingredients::add_standard_dynamic_body(step, crate, pos, true);
		crate.get<components::fixtures>().set_restitution(0.8f);
		crate.get<components::fixtures>().set_density(0.7f);
		crate.get<components::fixtures>().set_physical_material(assets::physical_material_id::WOOD);
		crate.add_standard_components(step);

		return crate;
	}

	entity_handle create_brick_wall(const logic_step step, const components::transform pos, vec2i size) {
		const auto crate = create_test_scene_entity(step.get_cosmos(), test_scene_type::BRICK_WALL);
		
		if (size.is_zero()) {
			size = step.get_logical_assets().at(assets::game_image_id::CRATE).get_size();
		}

		ingredients::add_standard_static_body(step, crate, pos);
		crate.get<components::fixtures>().set_restitution(0.0f);
		crate.get<components::fixtures>().set_density(100);
		crate.get<components::fixtures>().set_physical_material(assets::physical_material_id::WOOD);
		crate.add_standard_components(step);

		return crate;
	}
}
