#pragma once
#include "game/cosmos/entity_id.h"
#include "application/setups/editor/resources/editor_typed_resource_id.h"

template <class R, class E>
struct editor_node_base {
	editor_typed_resource_id<R> resource_id;
	E editable;

	bool visible = true;

	mutable entity_id scene_entity_id;
	mutable bool passed_filter = false;

	std::string unique_name;

	const auto& get_display_name() const {
		return unique_name;
	}
};
