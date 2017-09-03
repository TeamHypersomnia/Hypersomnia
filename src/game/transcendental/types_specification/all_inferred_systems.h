#pragma warning(disable : 4503)
#pragma once

#include "game/systems_inferred/tree_of_npo_system.h"
#include "game/systems_inferred/physics_system.h"
#include "game/systems_inferred/processing_lists_system.h"
#include "game/systems_inferred/relational_system.h"
#include "game/systems_inferred/name_system.h"

namespace augs {
	template <class...>
	class storage_for_systems;
}

using all_systems_inferred = augs::storage_for_systems<
	// It is critical that the relational system is the first on this list
	// so that it creates inferred state of relations before physics_system uses it for constructing
	// bodies, fixtures and joints.
	relational_system,
	name_system,
	physics_system,
	tree_of_npo_system,
	processing_lists_system
>;