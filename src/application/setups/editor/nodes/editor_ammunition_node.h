#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "application/setups/editor/nodes/editor_node_base.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/resources/editor_ammunition_resource.h"

struct editor_ammunition_node_editable {
	// GEN INTROSPECTOR struct editor_ammunition_node_editable
	vec2 pos;
	real32 rotation = 0.0f;
	// END GEN INTROSPECTOR
};

struct editor_ammunition_node : editor_node_base<
	editor_ammunition_resource,
	editor_ammunition_node_editable
> {
	auto get_transform() const {
		return transformr(editable.pos, editable.rotation);
	}

	static const char* get_type_name() {
		return "Ammunition";
	}
};
