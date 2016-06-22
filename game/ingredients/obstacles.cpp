#include "ingredients.h"
#include "game/cosmos.h"
#include "game/components/name_component.h"

namespace prefabs {
	entity_handle create_crate(cosmos world, vec2 pos, vec2 size) {
		auto crate = world.create_entity("crate");

		name_entity(crate, entity_name::CRATE);
		ingredients::sprite_scalled(crate, pos, size, assets::texture_id::CRATE, augs::white, render_layer::DYNAMIC_BODY);
		ingredients::standard_dynamic_body(crate);

		return crate;
	}
}
