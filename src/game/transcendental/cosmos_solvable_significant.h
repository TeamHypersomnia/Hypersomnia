#pragma once
#include "augs/misc/constant_size_vector.h"
#include "augs/misc/pool/pool.h"

#include "augs/templates/get_by_dynamic_id.h"

#include "game/transcendental/entity_pools.h"
#include "game/transcendental/cosmos_clock.h"
#include "game/transcendental/entity_solvable.h"

struct cosmos_solvable_significant {
	// GEN INTROSPECTOR struct cosmos_solvable_significant
	all_entity_pools entity_pools;
	cosmos_clock clock;
	// END GEN INTROSPECTOR

	template <class E>
	auto& get_pool() {
		return std::get<make_entity_pool<E>>(entity_pools);
	}

	template <class E>
	const auto& get_pool() const {
		return std::get<make_entity_pool<E>>(entity_pools);
	}

	template <class F>
	decltype(auto) on_pool(const entity_type_id id, F&& callback) {
		return get_by_dynamic_index(entity_pools, id.get_index(), std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) on_pool(const entity_type_id id, F&& callback) const {
		return get_by_dynamic_index(entity_pools, id.get_index(), std::forward<F>(callback));
	}

	void clear();
};