#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/resources/editor_light_resource.h"

struct editor_light_node_editable {
	// GEN INTROSPECTOR struct editor_light_node_editable
	vec2 pos;
	real32 scale_intensity = 0.0f;
	// END GEN INTROSPECTOR
};

struct editor_light_node {
	editor_typed_resource_id<editor_light_resource> resource_id;
	editor_light_node_editable editable;
	bool visible = true;

	auto get_transform() const {
		return transformr(editable.pos, 0.0f);
	}

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}
};
