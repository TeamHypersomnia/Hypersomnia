#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "game/cosmos/entity_id.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/resources/editor_marker_resource.h"
#include "game/enums/marker_type.h"
#include "game/enums/faction_type.h"

struct editor_point_marker_node_editable {
	// GEN INTROSPECTOR struct editor_point_marker_node_editable
	faction_type faction = faction_type::RESISTANCE;
	marker_letter_type letter = marker_letter_type::A;

	vec2 pos;
	real32 rotation = 0.0f;
	// END GEN INTROSPECTOR
};

struct editor_area_marker_node_editable {
	// GEN INTROSPECTOR struct editor_area_marker_node_editable
	faction_type faction = faction_type::RESISTANCE;
	marker_letter_type letter = marker_letter_type::A;

	vec2 pos;
	real32 rotation = 0.0f;
	augs::maybe<vec2i> size;
	// END GEN INTROSPECTOR
};

struct editor_point_marker_node {
	editor_typed_resource_id<editor_point_marker_resource> resource_id;
	editor_point_marker_node_editable editable;

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
		return "Area marker";
	}
};

struct editor_area_marker_node {
	editor_typed_resource_id<editor_area_marker_resource> resource_id;
	editor_area_marker_node_editable editable;

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
		return "Point marker";
	}
};
