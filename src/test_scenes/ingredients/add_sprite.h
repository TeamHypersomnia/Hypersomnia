#pragma once
#include "game/assets/all_logical_assets.h"

#include "game/components/sprite_component.h"
#include "game/components/render_component.h"

#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"

#include "game/enums/filters.h"
#include "view/viewables/image_cache.h"
#include "test_scenes/test_scene_images.h"
#include "test_scenes/test_sorting_orders.h"

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
			const auto layer,
			const rgba color = white,
			const augs::sprite_special_effect effect = augs::sprite_special_effect::NONE,
			const float effect_speed = 1.f
		) -> auto& {
			auto& meta = get_test_flavour(flavours, flavour_id);

			invariants::render render_def;
			render_def.layer = get_layer_for_order_type(layer);
			meta.set(render_def);

			if constexpr(!std::is_same_v<const render_layer, decltype(layer)>) {
				invariants::sorting_order order_def;
				order_def.order = static_cast<sorting_order_type>(layer);
				meta.set(order_def);
			}

			test_flavours::add_sprite(meta, caches, image_id, color, effect).effect_speed_multiplier = effect_speed;

			return meta;
		};

		return lbd;
	}

}