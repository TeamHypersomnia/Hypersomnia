#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "game/cosmos/entity_id.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/resources/editor_explosive_resource.h"

struct editor_explosive_node_editable {
	// GEN INTROSPECTOR struct editor_explosive_node_editable
	vec2 pos;
	real32 rotation = 0.0f;
	// END GEN INTROSPECTOR
};

struct editor_explosive_node {
	editor_typed_resource_id<editor_explosive_resource> resource_id;
	editor_explosive_node_editable editable;

	bool visible = true;

	mutable entity_id scene_entity_id;

	auto get_transform() const {
		return transformr(editable.pos, editable.rotation);
	}

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}

	static const char* get_type_name() {
		return "explosive";
	}
};
