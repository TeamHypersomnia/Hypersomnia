#pragma once
#include "game/assets/all_logical_assets.h"

#include "game/components/sprite_component.h"
#include "game/components/render_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/enums/filters.h"

namespace test_flavours {
	template <class E>
	void add_sprite(
		E& t, 
		const loaded_image_caches_map& logicals,
		const assets::image_id id, 
		const rgba col = white,
		const augs::sprite_special_effect effect = augs::sprite_special_effect::NONE
	) {
		invariants::sprite sprite_def;
		sprite_def.set(id, logicals, col);
		sprite_def.effect = effect;
		t.set(sprite_def);
	}

	template <class E>
	void add_sprite(
		E& t, 
		const loaded_image_caches_map& logicals,
		const test_scene_image_id id, 
		const rgba col = white,
		const augs::sprite_special_effect effect = augs::sprite_special_effect::NONE
	) {
		add_sprite(t, logicals, to_image_id(id), col, effect); 
	}
}