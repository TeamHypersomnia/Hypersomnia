#pragma once
#include "game/assets/asset_pools.h"

#include "game/assets/ids/asset_ids.h"

#include "game/assets/recoil_player.h"
#include "game/assets/animation.h"
#include "game/assets/physical_material.h"

using plain_animations_pool = make_asset_pool<plain_animation, assets::plain_animation_id_key>;
using torso_animations_pool = make_asset_pool<torso_animation, assets::torso_animation_id_key>;
using legs_animations_pool = make_asset_pool<legs_animation, assets::legs_animation_id_key>;
using recoil_players_pool = make_asset_pool<recoil_player, assets::recoil_player_id_key>;
using physical_materials_pool = make_asset_pool<physical_material, assets::physical_material_id_key>;

struct all_logical_assets {
	// GEN INTROSPECTOR struct all_logical_assets
	plain_animations_pool plain_animations;
	torso_animations_pool torso_animations;
	legs_animations_pool legs_animations;

	recoil_players_pool recoils;
	physical_materials_pool physical_materials;
	// END GEN INTROSPECTOR
};
