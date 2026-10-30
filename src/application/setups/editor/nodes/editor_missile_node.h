#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "application/setups/editor/nodes/editor_node_base.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/resources/editor_missile_resource.h"

/*
	Fired once, as the round starts: flies from pos in the direction of rotation.
*/

struct editor_missile_node_editable {
	// GEN INTROSPECTOR struct editor_missile_node_editable
	vec2 pos;
	real32 rotation = 0.0f;
	real32 speed = 4000.0f;
	// END GEN INTROSPECTOR
};

struct editor_missile_node : editor_node_base<
	editor_missile_resource,
	editor_missile_node_editable
> {
	auto get_transform() const {
		return transformr(editable.pos, editable.rotation);
	}

	static const char* get_type_name() {
		return "Missile";
	}
};
