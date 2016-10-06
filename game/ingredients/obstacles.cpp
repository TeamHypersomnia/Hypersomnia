#include "ingredients.h"
#include "game/transcendental/cosmos.h"
#include "game/components/name_component.h"

namespace prefabs {
	entity_handle create_crate(cosmos& world, const components::transform pos, const vec2 size) {
		const auto crate = world.create_entity("crate");

		name_entity(crate, entity_name::CRATE);
		ingredients::sprite_scalled(crate, pos, size, assets::texture_id::CRATE, augs::white, render_layer::DYNAMIC_BODY);
		ingredients::standard_dynamic_body(crate, true);

		crate.add_standard_components();

		return crate;
	}
}
