#pragma once
#include <unordered_set>
#include <unordered_map>

#include "game/transcendental/per_entity_type.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "game/transcendental/entity_flavour_id.h"

namespace components {
	struct type;
}

class flavour_id_cache {
	using caches_type = per_entity_type_array<
		std::unordered_map<
			raw_entity_flavour_id, 
			std::unordered_set<entity_id_base>
		>
	>;

	caches_type caches;

	auto& get_entities_by_flavour_map(const entity_id id) {
		return caches[id.type_id.get_index()];
	}

	const auto& get_entities_by_flavour_map(const entity_flavour_id id) const {
		return caches[id.type_id.get_index()];
	}

	const std::unordered_set<entity_id_base>& detail_get_entities_by_flavour_id(const entity_flavour_id) const;

public:
	/* TODO: templatize whole cache? */

	template <class E>
	const auto& get_entities_by_flavour_id(const typed_entity_flavour_id<E> id) const {
		using return_type = const std::unordered_set<typed_entity_id<E>>&;
		const auto& result = detail_get_entities_by_flavour_id(entity_flavour_id(id));

		/* YOLO cast */
		return reinterpret_cast<return_type>(result);
	}

	void infer_cache_for(const const_entity_handle);
	void destroy_cache_of(const const_entity_handle);
};