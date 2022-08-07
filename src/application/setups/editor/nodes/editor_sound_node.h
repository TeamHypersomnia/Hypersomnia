#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "game/cosmos/entity_id.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "game/detail/view_input/sound_effect_modifier.h"

struct editor_sound_resource;

struct editor_sound_node_editable : sound_effect_modifier {
	using base = sound_effect_modifier;
	using introspect_base = base;

	// GEN INTROSPECTOR struct editor_sound_node_editable
	vec2 pos;
	// END GEN INTROSPECTOR
};

struct editor_sound_node {
	editor_typed_resource_id<editor_sound_resource> resource_id;
	editor_sound_node_editable editable;
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
		return "Sound";
	}
};
