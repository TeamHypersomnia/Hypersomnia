#pragma once
#include "game/cosmos/entity_id.h"
#include "application/setups/editor/nodes/editor_typed_node_id.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"

struct editor_prefab_node;

template <class R, class E>
struct editor_node_base {
	using resource_type = R;
	using base = editor_node_base<R, E>;

	editor_typed_resource_id<R> resource_id;
	E editable;

	bool active = true;

	mutable entity_id scene_entity_id;
	mutable bool passed_filter = false;

	std::string unique_name;

	void clear_duplicated_fields() {
		scene_entity_id.unset();
	}

	const auto& get_display_name() const {
		return unique_name;
	}
};
