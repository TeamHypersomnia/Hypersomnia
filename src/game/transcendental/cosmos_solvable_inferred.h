#pragma warning(disable : 4503)
#pragma once

#include <tuple>
#include "augs/templates/for_each_std_get.h"

#include "game/inferred_caches/tree_of_npo_cache.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/inferred_caches/processing_lists_cache.h"
#include "game/inferred_caches/relational_cache.h"
#include "game/inferred_caches/type_id_cache.h"

struct cosmos_solvable_inferred {
	relational_cache relational;
	type_id_cache name;
	physics_world_cache physics;
	tree_of_npo_cache tree_of_npo;
	processing_lists_cache processing_lists;

	auto tie() {
		return std::tie(
			relational,
			name,
			physics,
			tree_of_npo,
			processing_lists
		);
	}

	auto tie() const {
		return std::tie(
			relational,
			name,
			physics,
			tree_of_npo,
			processing_lists
		);
	}

	template <class F>
	void for_each(F&& f) {
		for_each_through_std_get(tie(), std::forward<F>(f));		
	}

	template <class F>
	void for_each(F&& f) const {
		for_each_through_std_get(tie(), std::forward<F>(f));		
	}
};