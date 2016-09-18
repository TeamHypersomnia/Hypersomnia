#include "ingredients.h"
#include "renderable_includes.h"
#include "game/enums/filters.h"

namespace ingredients {
	components::sprite& sprite(const entity_handle e, const components::transform pos, const assets::texture_id id, const augs::rgba col, const render_layer layer) {
		auto& sprite = e += components::sprite();
		auto& render = e += components::render();
		auto& transform = e += pos;

		render.layer = layer;
		sprite.set(id, col);

		return sprite;
	}

	components::sprite&  sprite_scalled(const entity_handle e, const components::transform pos, const vec2i size, const assets::texture_id id, const augs::rgba col, const render_layer layer) {
		sprite(e, pos, id, col, layer);
		e.get<components::sprite>().size = size;
		return e.get<components::sprite>();
	}
}