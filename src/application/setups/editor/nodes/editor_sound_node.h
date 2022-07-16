#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"

struct editor_sound_resource;

struct editor_sound_node_editable {
	// GEN INTROSPECTOR struct editor_sound_node_editable
	vec2 pos;
	// END GEN INTROSPECTOR
};

struct editor_sound_node {
	editor_typed_resource_id<editor_sound_resource> resource_id;
	editor_sound_node_editable editable;
	bool visible = true;

	auto get_transform() const {
		return transformr(editable.pos, 0.0f);
	}

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}
};
