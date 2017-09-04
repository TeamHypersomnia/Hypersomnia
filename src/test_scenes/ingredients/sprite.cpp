#include "ingredients.h"

#include "game/assets/all_assets.h"

#include "game/components/sprite_component.h"
#include "game/components/render_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/enums/filters.h"

namespace ingredients {
	components::sprite& add_sprite(
		const all_logical_assets& metas,
		const entity_handle e,
		const assets::game_image_id id, 
		const rgba col, 
		const render_layer layer
	) {
		auto& sprite = e += components::sprite();
		auto& render = e += components::render();

		render.layer = layer;
		sprite.set(id, metas, col);

		return sprite;
	}

	components::sprite& add_sprite_scaled(
		const entity_handle e, 
		const vec2i size, 
		const assets::game_image_id id, 
		const rgba col, 
		const render_layer layer
	) {
		ensure(size.non_zero());

		auto& sprite = e += components::sprite();
		auto& render = e += components::render();

		render.layer = layer;
		sprite.set(id, size, col);

		return sprite;
	}
}