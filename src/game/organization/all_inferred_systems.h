#pragma warning(disable : 4503)
#pragma once

#include "game/inferential_systems/tree_of_npo_system.h"
#include "game/inferential_systems/physics_system.h"
#include "game/inferential_systems/processing_lists_system.h"
#include "game/inferential_systems/relational_system.h"
#include "game/inferential_systems/name_system.h"

namespace augs {
	template <class...>
	class storage_for_systems;
}

using all_inferential_systems = augs::storage_for_systems<
	// It is critical that the relational system is the first on this list
	// so that it creates inferred state of relations before physics_system uses it for constructing
	// bodies, fixtures and joints.
	relational_system,
	name_system,
	physics_system,
	tree_of_npo_system,
	processing_lists_system
>;