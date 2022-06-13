#pragma once
#include "augs/math/vec2.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"

struct editor_prefab_resource;

struct editor_prefab_node {
	// GEN INTROSPECTOR struct editor_prefab_node
	editor_typed_resource_id<editor_prefab_resource> resource_id;

	vec2 pos;
	real32 rotation = 0.0f;
	// END GEN INTROSPECTOR
};
