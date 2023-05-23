#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "application/setups/editor/nodes/editor_node_base.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/resources/editor_marker_resource.h"
#include "game/enums/marker_type.h"
#include "game/enums/faction_type.h"
#include "application/setups/editor/nodes/editor_marker_node_editable.h"

struct editor_point_marker_node : editor_node_base<
	editor_point_marker_resource,
	editor_point_marker_node_editable
> {
	auto get_transform() const {
		return transformr(editable.pos, editable.rotation);
	}

	static const char* get_type_name() {
		return "Area marker";
	}
};

struct editor_area_marker_node : editor_node_base<
	editor_area_marker_resource,
	editor_area_marker_node_editable
> {
	auto get_transform() const {
		return transformr(editable.pos, editable.rotation);
	}

	static const char* get_type_name() {
		return "Point marker";
	}
};
