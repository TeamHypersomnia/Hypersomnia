#pragma once
#include "augs/templates/folded_finders.h"
#include "game/assets/ids/asset_ids.h"

template <class T>
constexpr bool is_asset_id_v = is_one_of_v<
	T,

	/* Logical */
	assets::animation_id,
	assets::recoil_player_id,
	assets::physical_material_id,

	/* Viewables */
	//assets::image_id,
	assets::particle_effect_id,
	assets::sound_id
>;

