#include "ingredients.h"
#include "renderable_includes.h"

#include "../detail/physics_setup_helpers.h"
#include "../globals/filters.h"

namespace ingredients {
	components::sprite&  sprite(augs::entity_id e, vec2 pos, assets::texture_id id, augs::rgba col, render_layer layer) {
		auto& sprite = *e += components::sprite();
		auto& render = *e += components::render();
		auto& transform = *e += components::transform();

		render.layer = layer;
		transform.pos = pos;
		sprite.set(id, col);

		return sprite;
	}

	components::sprite&  sprite_scalled(augs::entity_id e, vec2 pos, vec2i size, assets::texture_id id, augs::rgba col, render_layer layer) {
		sprite(e, pos, id, col, layer);
		e->get<components::sprite>().size = size;
		return e->get<components::sprite>();
	}
}