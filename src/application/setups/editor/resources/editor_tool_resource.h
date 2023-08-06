#pragma once
#include "augs/math/vec2.h"
#include "game/cosmos/entity_flavour_id.h"
#include "test_scenes/test_scene_flavour_ids.h"

using ad_hoc_entry_id = uint32_t;

struct editor_tool_resource_editable {
	// GEN INTROSPECTOR struct editor_tool_resource_editable
	bool dummy = false;
	// END GEN INTROSPECTOR

	bool operator==(const editor_tool_resource_editable&) const = default;
};

struct editor_tool_node;
struct editor_tool_resource {
	using node_type = editor_tool_node;

	editor_tool_resource_editable editable;

	std::optional<std::variant<
		test_tool_items
	>> official_tag;

	ad_hoc_entry_id thumbnail_id = static_cast<ad_hoc_entry_id>(-1);

	mutable std::variant<
		typed_entity_flavour_id<tool_item>,
		typed_entity_flavour_id<container_item>
	> scene_flavour_id;

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}

	static const char* get_type_name() {
		return "tool";
	}
};
