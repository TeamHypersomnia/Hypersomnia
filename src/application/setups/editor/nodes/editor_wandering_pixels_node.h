#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "application/setups/editor/nodes/editor_node_base.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/resources/editor_wandering_pixels_resource.h"
#include "game/components/wandering_pixels_component.h"

struct editor_wandering_pixels_node_editable : components::wandering_pixels {
	using introspect_base = components::wandering_pixels;

	// GEN INTROSPECTOR struct editor_wandering_pixels_node_editable
	vec2 pos;
	real32 rotation = 0.0f;
	vec2i size;
	// END GEN INTROSPECTOR
};

struct editor_wandering_pixels_node : editor_node_base<
	editor_wandering_pixels_resource,
	editor_wandering_pixels_node_editable
> {
	auto get_transform() const {
		return transformr(editable.pos, editable.rotation);
	}

	static const char* get_type_name() {
		return "Wandering pixels";
	}
};
