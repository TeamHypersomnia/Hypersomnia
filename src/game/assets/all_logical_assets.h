#pragma once
#include "game/assets/asset_pools.h"

#include "game/assets/ids/asset_ids.h"

#include "game/assets/recoil_player.h"
#include "game/assets/animation.h"
#include "game/assets/physical_material.h"
#include "game/assets/image_offsets.h"

#include "game/assets/get_logicals_pool.h"

using plain_animations_pool = make_asset_pool<plain_animation, assets::plain_animation_id_key>;
using recoil_players_pool = make_asset_pool<recoil_player, assets::recoil_player_id_key>;
using physical_materials_pool = make_asset_pool<physical_material, assets::physical_material_id_key>;

using all_image_offsets_array_type = std::array<all_image_offsets, MAX_IMAGES_IN_INTERCOSM>;

struct all_logical_assets {
	// GEN INTROSPECTOR struct all_logical_assets
	plain_animations_pool plain_animations;

	recoil_players_pool recoils;
	physical_materials_pool physical_materials;
	// END GEN INTROSPECTOR

	all_image_offsets_array_type image_offsets;

	auto& get_offsets(const assets::image_id id) {
		return image_offsets.at(id.get_cache_index());
	}

	const auto& get_offsets(const assets::image_id id) const {
		return image_offsets.at(id.get_cache_index());
	}

	template <class T>
	auto find(const T& id) {
		return mapped_or_nullptr(get_logicals_pool<T>(*this), id);
	}

	template <class T>
	auto find(const T& id) const {
		return mapped_or_nullptr(get_logicals_pool<T>(*this), id);
	}
};
