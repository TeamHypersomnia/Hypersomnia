#include "ingredients.h"
#include "renderable_includes.h"
#include "game/enums/filters.h"

namespace ingredients {
	components::sprite& sprite(entity_handle e, vec2 pos, assets::texture_id id, augs::rgba col, render_layer layer) {
		auto& sprite = e += components::sprite();
		auto& render = e += components::render();
		auto& transform = e += components::transform();

		render.layer = layer;
		transform.pos = pos;
		sprite.set(id, col);

		return sprite;
	}

	components::sprite&  sprite_scalled(entity_handle e, vec2 pos, vec2i size, assets::texture_id id, augs::rgba col, render_layer layer) {
		sprite(e, pos, id, col, layer);
		e.get<components::sprite>().size = size;
		return e.get<components::sprite>();
	}
}