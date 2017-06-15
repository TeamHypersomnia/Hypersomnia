#include "ingredients.h"
#include "game/components/sprite_component.h"
#include "game/components/render_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/enums/filters.h"

namespace ingredients {
	components::sprite& add_sprite(
		const entity_handle e, 
		const assets::game_image_id id, 
		const rgba col, 
		const render_layer layer
	) {
		auto& sprite = e += components::sprite();
		auto& render = e += components::render();

		render.layer = layer;
		sprite.set(id, col);

		return sprite;
	}

	components::sprite& add_sprite_scaled(
		const entity_handle e, 
		const vec2i size, 
		const assets::game_image_id id, 
		const rgba col, 
		const render_layer layer
	) {
		add_sprite(e, id, col, layer);
		
		if (size.non_zero()) {
			e.get<components::sprite>().overridden_size = size;
		}

		return e.get<components::sprite>();
	}
}