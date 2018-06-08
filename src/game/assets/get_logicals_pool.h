#pragma once
#include "augs/templates/identity_templates.h"
#include "game/assets/ids/asset_ids.h"

template <class I, class T>
decltype(auto) get_logicals_pool(T&& t) {
	if constexpr(std::is_same_v<I, assets::recoil_player_id>) {
		return (t.recoils);
	}
	else if constexpr(std::is_same_v<I, assets::physical_material_id>) {
		return (t.physical_materials);
	}
	else if constexpr(std::is_same_v<I, assets::plain_animation_id>) {
		return (t.plain_animations);
	}
	else if constexpr(std::is_same_v<I, assets::torso_animation_id>) {
		return (t.torso_animations);
	}
	else if constexpr(std::is_same_v<I, assets::legs_animation_id>) {
		return (t.legs_animations);
	}
	else {
		return always_false<I>();
	}
}
