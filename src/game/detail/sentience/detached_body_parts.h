#pragma once
#include "game/cosmos/entity_id.h"
#include "game/cosmos/entity_flavour_id.h"

using body_part_flavour = constrained_entity_flavour_id<
	invariants::rigid_body
>;

template <class T>
struct basic_detached_body_parts {
	// GEN INTROSPECTOR struct basic_detached_body_parts class T
	T head;
	// END GEN INTROSPECTOR
};

using detached_body_parts = basic_detached_body_parts<entity_id>;
using detached_body_parts_flavours = basic_detached_body_parts<body_part_flavour>;
