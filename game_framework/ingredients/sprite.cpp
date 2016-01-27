#include "ingredients.h"
#include "renderable_includes.h"

#include "../shared/physics_setup_helpers.h"
#include "../globals/filters.h"

namespace ingredients {
	void sprite(augs::entity_id e, vec2 pos, assets::texture_id id, augs::rgba col, render_layer layer) {
		auto& sprite = *e += components::sprite();
		auto& render = *e += components::render();
		auto& transform = *e += components::transform();

		render.layer = layer;
		transform.pos = pos;
		sprite.set(id, col);
	}

	void sprite_scalled(augs::entity_id e, vec2 pos, vec2i size, assets::texture_id id, augs::rgba col, render_layer layer) {
		sprite(e, pos, id, col, layer);
		e->get<components::sprite>().size = size;
	}
}