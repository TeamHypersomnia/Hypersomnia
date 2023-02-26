#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "game/cosmos/entity_id.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/resources/editor_light_resource.h"

#include "application/setups/editor/nodes/editor_light_falloff.h"

struct editor_light_node_editable {
	// GEN INTROSPECTOR struct editor_light_node_editable
	vec2 pos;
	rgba colorize = white;

	editor_light_falloff falloff;
	augs::maybe<editor_light_falloff> wall_falloff;

	float positional_vibration = 1.0f;
	float intensity_vibration = 1.0f;
	// END GEN INTROSPECTOR
};

struct editor_light_node {
	editor_typed_resource_id<editor_light_resource> resource_id;
	editor_light_node_editable editable;
	bool visible = true;

	mutable entity_id scene_entity_id;

	auto get_transform() const {
		return transformr(editable.pos, 0.0f);
	}

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}

	static const char* get_type_name() {
		return "Light";
	}
};
