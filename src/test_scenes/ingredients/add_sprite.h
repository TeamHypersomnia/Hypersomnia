#pragma once
#include "game/assets/all_logical_assets.h"

#include "game/components/sprite_component.h"
#include "game/components/render_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

#include "game/enums/filters.h"
#include "view/viewables/image_cache.h"
#include "test_scenes/test_scene_images.h"

namespace test_flavours {
	template <class E>
	auto& add_sprite(
		E& t, 
		const loaded_image_caches_map& caches,
		const assets::image_id id, 
		const rgba col = white,
		const augs::sprite_special_effect effect = augs::sprite_special_effect::NONE
	) {
		invariants::sprite sprite_def;
		sprite_def.set(id, caches, col);
		sprite_def.effect = effect;
		t.set(sprite_def);
		return t.template get<invariants::sprite>();
	}

	template <class E>
	auto& add_sprite(
		E& t, 
		const loaded_image_caches_map& caches,
		const test_scene_image_id id, 
		const rgba col = white,
		const augs::sprite_special_effect effect = augs::sprite_special_effect::NONE
	) {
		return add_sprite(t, caches, to_image_id(id), col, effect); 
	}

	template <class T, class C>
	auto flavour_with_sprite_maker(T& flavours, C& caches) {
		auto lbd = [&](
			const auto flavour_id,
			const auto image_id,
			const render_layer layer,
			const rgba color = white,
			const augs::sprite_special_effect effect = augs::sprite_special_effect::NONE
		) -> auto& {
			auto& meta = get_test_flavour(flavours, flavour_id);

			{
				invariants::render render_def;
				render_def.layer = layer;
				meta.set(render_def);
			}

			test_flavours::add_sprite(meta, caches, image_id, color, effect);

			return meta;
		};

		return lbd;
	}

}