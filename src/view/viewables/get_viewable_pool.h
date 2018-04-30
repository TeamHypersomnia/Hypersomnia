#pragma once
#include "augs/templates/identity_templates.h"
#include "game/assets/ids/asset_ids.h"

template <class I, class T>
auto& get_viewable_pool(T&& t) {
	if constexpr(std::is_same_v<I, assets::image_id>) {
		return t.image_definitions;
	}
	else if constexpr(std::is_same_v<I, assets::sound_id>) {
		return t.sounds;
	}
	else if constexpr(std::is_same_v<I, assets::particle_effect_id>) {
		return t.particle_effects;
	}
	else {
		static_assert(always_false_v<I>, "Unimplemented id type.");
	}
}

