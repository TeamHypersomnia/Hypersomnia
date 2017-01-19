#include "ingredients.h"
#include "game/transcendental/cosmos.h"
#include "game/components/name_component.h"
#include "game/components/fixtures_component.h"

namespace prefabs {
	entity_handle create_crate(cosmos& world, const components::transform pos, const vec2 size) {
		const auto crate = world.create_entity("crate");

		name_entity(crate, entity_name::CRATE);
		ingredients::sprite_scalled(crate, pos, size, assets::texture_id::CRATE, white, render_layer::DYNAMIC_BODY);
		ingredients::standard_dynamic_body(crate, true);
		crate.get<components::fixtures>().set_restitution(0.8f);
		crate.get<components::fixtures>().set_density(0.03f);
		crate.add_standard_components();

		return crate;
	}

	entity_handle create_brick_wall(cosmos& world, const components::transform pos, const vec2 size) {
		const auto crate = world.create_entity("brick_wall");

		ingredients::sprite_scalled(crate, pos, size, assets::texture_id::BRICK_WALL, white, render_layer::DYNAMIC_BODY);
		ingredients::standard_static_body(crate);
		crate.get<components::fixtures>().set_restitution(0.0f);
		crate.get<components::fixtures>().set_density(100);
		crate.add_standard_components();

		return crate;
	}
}
