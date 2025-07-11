#pragma once
#include <cstddef>
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/official/official_resource_id_enums.h"

using to_resource_type_t = type_map<
	type_pair<official_sprites, editor_sprite_resource>,
	type_pair<official_sounds, editor_sound_resource>,
	type_pair<official_lights, editor_light_resource>,
	type_pair<official_particles, editor_particles_resource>,
	type_pair<official_materials, editor_material_resource>
>;

template <class P, class T>
inline P to_pool_id(const T id) {
	/* 
		This is a massive sleight of hand. 
		We predict here what identificators will be returned by the pool.
	*/

	P result;
	result.indirection_index = static_cast<unsigned>(id);
	result.version = 1;
	return result;
}

template <class T>
auto enum_count(const T) {
	static_assert(std::is_enum_v<T>);
	return static_cast<std::size_t>(T::COUNT);
}

template <class T>
auto to_resource_id(const T id) {
	using resource_type = typename to_resource_type_t::at<T>;
	const auto raw = to_pool_id<editor_resource_pool_id>(id);
	return editor_typed_resource_id<resource_type>::from_raw(raw, true);
}
