#pragma once
#include "game/cosmos/per_entity_type.h"
#include "augs/misc/constant_size_vector.h"

template <class T, class Value>
struct dummy_holder {
	Value value;
};

template <class Value>
struct make_linear_cache {
	template <class T>
	using type = std::conditional_t<
		statically_allocate_entities,
		dummy_holder<T, std::array<Value, T::statically_allocated_entities>>,
		dummy_holder<T, std::vector<Value>>
	>;
};

template <class cache_type, class EntityTypes>
using linear_cache_maps = per_type_container<EntityTypes, make_linear_cache<cache_type>::template type>;

template <class T, class EntityTypes>
struct linear_cache_map {
	linear_cache_maps<T, EntityTypes> all;

	void clear() {}
	void reserve(const std::size_t) {
		static_assert(statically_allocate_entities, "Unimplemented for non-static");
	}
};
