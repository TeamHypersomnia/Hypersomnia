#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "game/cosmos/entity_id.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"

struct editor_prefab_resource;

struct editor_prefab_node {
	// GEN INTROSPECTOR struct editor_prefab_node
	editor_typed_resource_id<editor_prefab_resource> resource_id;
	bool visible = true;

	vec2 pos;
	real32 rotation = 0.0f;
	// END GEN INTROSPECTOR

	mutable entity_id scene_entity_id;

	auto get_transform() const {
		return transformr(pos, rotation);
	}

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}
};
