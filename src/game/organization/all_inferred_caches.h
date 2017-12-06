#pragma warning(disable : 4503)
#pragma once

#include "game/inferred_caches/tree_of_npo_cache.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/inferred_caches/processing_lists_cache.h"
#include "game/inferred_caches/relational_cache.h"
#include "game/inferred_caches/name_cache.h"

struct all_inferred_caches {
	relational_cache relational;
	name_cache name;
	physics_world_cache physics;
	tree_of_npo_cache tree_of_npo;
	processing_lists_cache processing_lists;

	template <class F>
	void for_each(F f) {
		f(relational);
		f(name);
		f(physics);
		f(tree_of_npo);
		f(processing_lists);
	}

	template <class F>
	void for_each(F f) const {
		f(relational);
		f(name);
		f(physics);
		f(tree_of_npo);
		f(processing_lists);
	}
};