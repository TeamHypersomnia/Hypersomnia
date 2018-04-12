#pragma once
#include "game/assets/asset_containers.h"

#include "game/assets/ids/asset_ids.h"
#include "game/assets/ids/physical_material_id.h"
#include "game/assets/ids/recoil_player_id.h"

#include "game/assets/recoil_player.h"
#include "game/assets/animation.h"
#include "game/assets/physical_material.h"

using recoil_players_pool = asset_map<assets::recoil_player_id, recoil_player>;
using physical_materials_pool = asset_map<assets::physical_material_id, physical_material>;

struct all_logical_assets {
	// GEN INTROSPECTOR struct all_logical_assets
	animations_pool animations;
	recoil_players_pool recoils;
	physical_materials_pool physical_materials;
	// END GEN INTROSPECTOR
};