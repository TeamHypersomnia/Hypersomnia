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
	T arm_upper;
	T arm_lower;
	T leg_upper;
	T leg_lower;
	T lying_corpse;
	// END GEN INTROSPECTOR
};

/*
	Lying corpse variants without legs, for one state of the arms.
	noleg lacks the top leg of the image, noleg_other the bottom one.
*/

struct lying_corpse_legless_flavours {
	// GEN INTROSPECTOR struct lying_corpse_legless_flavours
	body_part_flavour noleg;
	body_part_flavour noleg_other;
	body_part_flavour nolegs;
	// END GEN INTROSPECTOR
};

using detached_body_parts = basic_detached_body_parts<entity_id>;
using detached_body_parts_flavours = basic_detached_body_parts<body_part_flavour>;
