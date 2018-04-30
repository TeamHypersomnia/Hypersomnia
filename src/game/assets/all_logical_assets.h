#pragma once
#include "game/assets/asset_pools.h"

#include "game/assets/ids/asset_ids.h"

#include "game/assets/recoil_player.h"
#include "game/assets/animation.h"
#include "game/assets/physical_material.h"

using animations_pool = animation_id_pool<animation>;
using recoil_players_pool = recoil_id_pool<recoil_player>;
using physical_materials_pool = physical_material_id_pool<physical_material>;

struct all_logical_assets {
	// GEN INTROSPECTOR struct all_logical_assets
	animations_pool animations;
	recoil_players_pool recoils;
	physical_materials_pool physical_materials;
	// END GEN INTROSPECTOR
};

template <class I, class T>
auto& get_logicals_pool(T&& t) {
	if constexpr(std::is_same_v<I, assets::recoil_player_id>) {
		return t.recoils;
	}
	else if constexpr(std::is_same_v<I, assets::physical_material_id>) {
		return t.physical_materials;
	}
	else {
		static_assert(always_false_v<I>, "Unimplemented id type.");
	}
}
