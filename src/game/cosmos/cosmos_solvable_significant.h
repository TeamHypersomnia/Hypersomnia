#pragma once
#include <unordered_map>
#include "augs/misc/constant_size_vector.h"
#include "augs/misc/pool/pool.h"

#include "augs/templates/get_by_dynamic_id.h"

#include "game/cosmos/entity_pools.h"
#include "game/cosmos/entity_solvable.h"

#include "game/common_state/entity_name_str.h"
#include "game/cosmos/entity_id.h"
#include "game/cosmos/cosmos_global_solvable.h"

#include "augs/misc/assignment_detector.h"

using cosmos_clock = augs::stepped_clock;

struct cosmos_solvable_significant {
	// GEN INTROSPECTOR struct cosmos_solvable_significant
	all_entity_pools entity_pools;
	cosmos_clock clk;
	entity_guid next_entity_guid = entity_guid::first();

	std::unordered_map<entity_id, entity_name_str> specific_names;
	cosmos_global_solvable global;
	// END GEN INTROSPECTOR

	mutable augs::assignment_detector assignment_detector;

	template <class E>
	auto& get_pool() {
		return entity_pools.get_for<E>();
	}

	template <class E>
	const auto& get_pool() const {
		return entity_pools.get_for<E>();
	}

	template <class F>
	decltype(auto) on_pool(const entity_type_id id, F&& callback) {
		return entity_pools.visit(id, std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) on_pool(const entity_type_id id, F&& callback) const {
		return entity_pools.visit(id, std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) for_each_entity_pool(F&& callback) {
		return entity_pools.for_each_container(std::forward<F>(callback));
	}

	template <class F>
	decltype(auto) for_each_entity_pool(F&& callback) const {
		return entity_pools.for_each_container(std::forward<F>(callback));
	}

	void clear();
};