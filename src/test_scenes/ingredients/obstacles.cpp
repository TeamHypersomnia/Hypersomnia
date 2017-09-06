#include "ingredients.h"
#include "game/assets/all_logical_assets.h"
#include "game/transcendental/cosmos.h"
#include "game/components/name_component.h"
#include "game/components/fixtures_component.h"

namespace prefabs {
	entity_handle create_crate(const logic_step step, const components::transform pos, vec2i size) {
		const auto crate = step.cosm.create_entity("crate");
		
		if (size.is_zero()) {
			size = step.input.logical_assets.at(assets::game_image_id::CRATE).get_size();
		}

		ingredients::add_sprite_scaled(crate, size, assets::game_image_id::CRATE, white, render_layer::DYNAMIC_BODY);

		ingredients::add_standard_dynamic_body(step, crate, pos, true);
		crate.get<components::fixtures>().set_restitution(0.8f);
		crate.get<components::fixtures>().set_density(0.7f);
		crate.get<components::fixtures>().set_physical_material(assets::physical_material_id::WOOD);
		crate.add_standard_components(step);

		return crate;
	}

	entity_handle create_brick_wall(const logic_step step, const components::transform pos, vec2i size) {
		const auto crate = step.cosm.create_entity("brick_wall");
		
		if (size.is_zero()) {
			size = step.input.logical_assets.at(assets::game_image_id::CRATE).get_size();
		}

		ingredients::add_sprite_scaled(crate, size, assets::game_image_id::BRICK_WALL, white, render_layer::DYNAMIC_BODY);
		ingredients::add_standard_static_body(step, crate, pos);
		crate.get<components::fixtures>().set_restitution(0.0f);
		crate.get<components::fixtures>().set_density(100);
		crate.get<components::fixtures>().set_physical_material(assets::physical_material_id::WOOD);
		crate.add_standard_components(step);

		return crate;
	}
}
