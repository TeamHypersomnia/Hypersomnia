#pragma once
#include "game/assets/asset_pools.h"

#include "game/assets/ids/asset_ids.h"

#include "game/assets/recoil_player.h"
#include "game/assets/animation.h"
#include "game/assets/physical_material.h"

using plain_animations_pool = plain_animation_pool<plain_animation>;
using recoil_players_pool = recoil_id_pool<recoil_player>;
using physical_materials_pool = physical_material_id_pool<physical_material>;

struct all_logical_assets {
	// GEN INTROSPECTOR struct all_logical_assets
	plain_animations_pool plain_animations;
	recoil_players_pool recoils;
	physical_materials_pool physical_materials;
	// END GEN INTROSPECTOR
};
