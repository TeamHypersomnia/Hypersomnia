#pragma once
#include "augs/templates/container_templates.h"
#include "augs/misc/convex_partitioned_shape.h"

#include "game/assets/asset_map.h"
#include "game/assets/all_logical_assets_declarations.h"

#include "game/assets/ids/game_image_id.h"
#include "game/assets/ids/animation_id.h"
#include "game/assets/ids/sound_buffer_id.h"
#include "game/assets/ids/particle_effect_id.h"
#include "game/assets/ids/physical_material_id.h"
#include "game/assets/ids/recoil_player_id.h"

#include "game/assets/recoil_player.h"
#include "game/assets/animation.h"
#include "game/assets/physical_material.h"

struct sound_buffer_logical {
	// GEN INTROSPECTOR struct sound_buffer_logical
	float max_duration_in_seconds = 0.f;
	unsigned num_of_variations = 0xdeadbeef;
	// END GEN INTROSPECTOR
};

struct game_image_logical {
	// GEN INTROSPECTOR struct game_image_logical
	vec2u original_image_size;
	convex_partitioned_shape shape;
	// END GEN INTROSPECTOR

	vec2u get_size() const {
		return original_image_size;
	}
};

struct particle_effect_logical {
	// GEN INTROSPECTOR struct particle_effect_logical
	float max_duration_ms;
	// END GEN INTROSPECTOR
};

#if STATICALLY_ALLOCATE_ASSETS
#include "augs/misc/trivially_copyable_tuple.h"

using tuple_of_logical_assets = augs::trivially_copyable_tuple<
	asset_map<assets::game_image_id, game_image_logical>,
	asset_map<assets::particle_effect_id, particle_effect_logical>,
	asset_map<assets::sound_buffer_id, sound_buffer_logical>,

	asset_map<assets::animation_id, animation>,
	asset_map<assets::recoil_player_id, recoil_player>,
	asset_map<assets::physical_material_id, physical_material>
>;
#else
#include <tuple>
using tuple_of_logical_assets = std::tuple<
	asset_map<assets::game_image_id, game_image_logical>,
	asset_map<assets::particle_effect_id, particle_effect_logical>,
	asset_map<assets::sound_buffer_id, sound_buffer_logical>,

	asset_map<assets::animation_id, animation>,
	asset_map<assets::recoil_player_id, recoil_player>,
	asset_map<assets::physical_material_id, physical_material>
>;
#endif

template <class T, class Candidate>
struct is_key_type_equal_to : std::bool_constant<std::is_same_v<T, typename Candidate::key_type>> {

};

template <class SearchedKeyType, class List>
using find_type_with_key_type_in_list_t = find_matching_type_in_list<bind_types<is_key_type_equal_to, SearchedKeyType>::template type, List>;

template <class SearchedKeyType, class... Types>
using find_type_with_key_type_t = find_type_with_key_type_in_list_t<SearchedKeyType, type_list<Types...>>;

template <class T, class ContainerList>
decltype(auto) get_container_with_key_type(ContainerList&& containers) {
	return std::get<
		find_type_with_key_type_in_list_t<
			T, 
			std::decay_t<ContainerList>
		>
	> (
		std::forward<ContainerList>(containers)
	);
}

struct all_logical_assets {
	// GEN INTROSPECTOR struct all_logical_assets
	tuple_of_logical_assets all;
	// END GEN INTROSPECTOR

	template <class T>
	auto& get_store_by(const T = T()) {
		return get_container_with_key_type<T>(all);
	}

	template <class T>
	const auto& get_store_by(const T = T()) const {
		return get_container_with_key_type<T>(all);
	}

	template <class T>
	decltype(auto) find(const T id) {
		return mapped_or_nullptr(get_store_by(id), id);
	}

	template <class T>
	decltype(auto) find(const T id) const {
		return mapped_or_nullptr(get_store_by(id), id);
	}

	template <class T>
	decltype(auto) operator[](const T id) {
		return get_store_by(id)[id];
	}

	template <class T>
	decltype(auto) at(const T id) {
		return get_store_by(id).at(id);
	}

	template <class T>
	decltype(auto) at(const T id) const {
		return get_store_by(id).at(id);
	}
};