#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "application/setups/editor/nodes/editor_node_base.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/resources/editor_sound_effect.h"

struct editor_sound_resource;

struct editor_sound_node_editable : editor_sound_property_effect_modifier {
	using base = editor_sound_property_effect_modifier;
	using introspect_base = base;

	// GEN INTROSPECTOR struct editor_sound_node_editable
	vec2 pos;
	// END GEN INTROSPECTOR
};

struct editor_sound_node : editor_node_base<
	editor_sound_resource,
	editor_sound_node_editable
> {
	auto get_transform() const {
		return transformr(editable.pos, 0.0f);
	}

	static const char* get_type_name() {
		return "Sound";
	}
};
