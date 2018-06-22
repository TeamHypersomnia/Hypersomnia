#pragma once
#include <unordered_set>
#include <unordered_map>

#include "game/transcendental/per_entity_type.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/transcendental/entity_flavour_id.h"

class flavour_id_cache {
	template <class T>
	using make_flavour_map = std::unordered_map<
		typed_entity_flavour_id<T>, 
		std::unordered_set<typed_entity_id<T>>
	>;

	using caches_type = per_entity_type_container<make_flavour_map>;

	caches_type caches;
public:
	template <class E>
	const auto& get_entities_by_flavour_id(const typed_entity_flavour_id<E> id) const {
		thread_local const std::unordered_set<typed_entity_id<E>> detail_none;

		if (const auto mapped = mapped_or_nullptr(caches.get_for<E>(), id)) {
			return *mapped;
		}

		return detail_none;
	}

	void infer_cache_for(const const_entity_handle);
	void destroy_cache_of(const const_entity_handle);
};