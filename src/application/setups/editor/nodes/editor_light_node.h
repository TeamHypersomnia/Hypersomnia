#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "application/setups/editor/nodes/editor_node_base.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/resources/editor_light_resource.h"

#include "application/setups/editor/nodes/editor_light_falloff.h"

struct editor_light_node_editable {
	// GEN INTROSPECTOR struct editor_light_node_editable
	vec2 pos;
	rgba colorize = white;

	editor_light_falloff falloff;
	augs::maybe<editor_light_falloff> wall_falloff;

	float positional_vibration = 0.5f;
	float intensity_vibration = 0.1f;
	// END GEN INTROSPECTOR
};

struct editor_light_node : editor_node_base<
	editor_light_resource,
	editor_light_node_editable
> {
	auto get_transform() const {
		return transformr(editable.pos, 0.0f);
	}

	static const char* get_type_name() {
		return "Light";
	}
};
