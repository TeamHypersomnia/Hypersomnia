#include "ingredients.h"
#include "entity_system/world.h"
#include "game_framework/components/name_component.h"

namespace prefabs {
	augs::entity_id create_crate(augs::world& world, vec2 pos, vec2 size) {
		auto crate = world.create_entity("crate");

		name_entity(crate, entity_name::CRATE);
		ingredients::sprite_scalled(crate, pos, size, assets::texture_id::CRATE, augs::white, render_layer::DYNAMIC_BODY);
		ingredients::crate_physics(crate);

		return crate;
	}
}
