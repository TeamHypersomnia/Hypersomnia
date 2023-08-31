#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "application/setups/editor/nodes/editor_node_base.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/resources/editor_marker_resource.h"
#include "application/setups/editor/nodes/editor_marker_node_editable.h"

struct editor_point_marker_node : editor_node_base<
	editor_point_marker_resource,
	editor_point_marker_node_editable
> {
	auto get_transform() const {
		return transformr(editable.pos, editable.rotation);
	}

	static const char* get_type_name() {
		return "Spot";
	}
};

struct editor_area_marker_node : editor_node_base<
	editor_area_marker_resource,
	editor_area_marker_node_editable
> {
	mutable std::optional<decltype(resource_type::scene_flavour_id)> custom_scene_flavour_id;

	auto get_transform() const {
		return transformr(editable.pos, editable.rotation);
	}

	static const char* get_type_name() {
		return "Zone";
	}
};
