#pragma once
#include "augs/templates/type_mod_templates.h"

#include "game/assets/ids/asset_ids.h"
#include "augs/misc/pool/pool.h"

template <class pooled_type, class... keys>
using make_asset_pool = augs::pool<pooled_type, make_vector, asset_pool_id_size_type, keys...>;


template <class pooled_type>
using image_id_pool = make_asset_pool<pooled_type, assets::image_id_key>;

template <class pooled_type>
using sound_id_pool = make_asset_pool<pooled_type, assets::sound_id_key>;

template <class pooled_type>
using plain_animation_pool = make_asset_pool<pooled_type, assets::plain_animation_id_key>;

template <class pooled_type>
using recoil_id_pool = make_asset_pool<pooled_type, assets::recoil_player_id_key>;

template <class pooled_type>
using physical_material_id_pool = make_asset_pool<pooled_type, assets::physical_material_id_key>;

template <class pooled_type>
using particle_effect_id_pool = make_asset_pool<pooled_type, assets::particle_effect_id_key>;
