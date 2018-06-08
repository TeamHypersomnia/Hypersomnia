#pragma once
#include "augs/templates/folded_finders.h"
#include "augs/misc/pool/pooled_object_id.h"

using asset_pool_id_size_type = unsigned short;

template <class key>
using make_asset_pool_id = augs::pooled_object_id<asset_pool_id_size_type, key>;

namespace assets {
	/* 
		Create keys so that asset ids designate separate types,
		thus forbidding them from being used interchangeably 
	*/

	class image_id_key { image_id_key() = delete; };
	class sound_id_key { sound_id_key() = delete; };
	class recoil_player_id_key { recoil_player_id_key() = delete; };
	class physical_material_id_key { physical_material_id_key() = delete; };
	class particle_effect_id_key { particle_effect_id_key() = delete; };

	class plain_animation_id_key { plain_animation_id_key() = delete; };

	using image_id = make_asset_pool_id<image_id_key>;
	using sound_id = make_asset_pool_id<sound_id_key>;

	using particle_effect_id = make_asset_pool_id<particle_effect_id_key>;
	using recoil_player_id = make_asset_pool_id<recoil_player_id_key>;
	using physical_material_id = make_asset_pool_id<physical_material_id_key>;

	using plain_animation_id = make_asset_pool_id<plain_animation_id_key>;

	using torso_animation_id = plain_animation_id;
	using legs_animation_id  = plain_animation_id;
}

template <class T>
constexpr bool is_pathed_asset = is_one_of_v<
	T, 
	assets::image_id, 
	assets::sound_id
>;

template <class T>
constexpr bool is_unpathed_asset = is_one_of_v<
	T, 
	assets::particle_effect_id, 
	assets::recoil_player_id,
	assets::physical_material_id,

	assets::plain_animation_id
>;

/* 
	Pathed assets are always viewables.
	It would be insane if the logic code had access to actual filesystem paths.
*/

template <class T>
constexpr bool is_viewable_asset_v = is_pathed_asset<T> || is_one_of_v<
	T,
	assets::particle_effect_id
>;

template <class T>
constexpr bool is_logical_asset_v = is_unpathed_asset<T> && !is_viewable_asset_v<T>;

template <class T, class = void>
struct has_image_id : std::false_type {};

template <class T>
struct has_image_id<T, decltype(std::declval<T&>().image_id, void())> : std::true_type {};

template <class T>
constexpr bool has_image_id_v = has_image_id<T>::value; 
