#pragma once
#include "augs/math/vec2.h"
#include "augs/math/transform.h"
#include "application/setups/editor/nodes/editor_node_base.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"
#include "application/setups/editor/resources/editor_prefab_resource.h"
#include "application/setups/editor/nodes/editor_prefab_node_editable.h"

struct editor_prefab_node : editor_node_base<
	editor_prefab_resource,
	editor_prefab_node_editable
> {
	auto get_transform() const {
		return transformr(editable.pos, editable.rotation);
	}

	static const char* get_type_name() {
		return "Prefab";
	}
};
