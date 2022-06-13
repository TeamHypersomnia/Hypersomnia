#pragma once
#include "augs/math/vec2.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/resources/editor_light_resource.h"

struct editor_light_node {
	// GEN INTROSPECTOR struct editor_light_node
	editor_typed_resource_id<editor_light_resource> resource_id;

	vec2 pos;
	real32 scale_intensity = 0.0f;
	// END GEN INTROSPECTOR
};
