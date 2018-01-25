#include "ingredients.h"

#include "game/assets/all_logical_assets.h"

#include "game/components/sprite_component.h"
#include "game/components/render_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/enums/filters.h"

namespace test_types {
	void add_sprite(
		entity_type& t, 
		const all_logical_assets& logicals,
		const assets::game_image_id id, 
		const rgba col,
		const invariants::sprite::special_effect effect
	) {
		invariants::sprite sprite_def;
		sprite_def.set(id, logicals, col);
		sprite_def.effect = effect;
		t.set(sprite_def);
	}
}