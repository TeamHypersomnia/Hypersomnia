#pragma once
#include <unordered_set>
#include <unordered_map>

#include "game/cosmos/per_entity_type.h"

#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_handle_declaration.h"

#include "game/cosmos/entity_flavour_id.h"

class cosmos;

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
	struct concerned_with {
		static constexpr bool value = true;
	};

	template <class E>
	const auto& get_entities_by_flavour_id(const typed_entity_flavour_id<E> id) const {
		thread_local const std::unordered_set<typed_entity_id<E>> detail_none;

		if (const auto mapped = mapped_or_nullptr(caches.get_for<E>(), id)) {
			return *mapped;
		}

		return detail_none;
	}

	template <class E>
	void specific_infer_cache_for(const E&);

	void infer_all(const cosmos&);

	void infer_cache_for(const const_entity_handle&);
	void destroy_cache_of(const const_entity_handle&);

	bool enabled = false;
};