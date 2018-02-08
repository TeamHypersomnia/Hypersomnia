#pragma once
#include "augs/misc/constant_size_vector.h"
#include "augs/misc/pool/pool.h"

#include "game/transcendental/cosmic_types.h"
#include "game/transcendental/cosmos_clock.h"

struct cosmos_solvable_significant {
	// GEN INTROSPECTOR struct cosmos_solvable_significant
	all_aggregate_pools aggregate_pools;
	cosmos_clock clock;
	// END GEN INTROSPECTOR

	template <class E>
	auto& get_pool() {
		return std::get<make_aggregate_pool<E>>(aggregate_pools);
	}

	template <class E>
	const auto& get_pool() const {
		return std::get<make_aggregate_pool<E>>(aggregate_pools);
	}

	template <class F>
	decltype(auto) on_pool(const entity_type_id id, F&& callback) {
		return get_by_dynamic_id(aggregate_pools, id.get_index(), std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) on_pool(const entity_type_id id, F&& callback) const {
		return get_by_dynamic_id(aggregate_pools, id.get_index(), std::forward<F>(callback));
	}

	void clear();
};