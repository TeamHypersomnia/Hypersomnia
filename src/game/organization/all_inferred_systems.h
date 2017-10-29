#pragma warning(disable : 4503)
#pragma once

#include "game/inferential_systems/tree_of_npo_system.h"
#include "game/inferential_systems/physics_system.h"
#include "game/inferential_systems/processing_lists_system.h"
#include "game/inferential_systems/relational_system.h"
#include "game/inferential_systems/name_system.h"

struct all_inferential_systems {
	relational_system relational;
	name_system name;
	physics_system physics;
	tree_of_npo_system tree_of_npo;
	processing_lists_system processing_lists;

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