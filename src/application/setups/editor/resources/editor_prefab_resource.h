#pragma once
#include "application/setups/editor/nodes/editor_node_id.h"
#include "application/setups/editor/nodes/editor_prefab_node_editable.h"

enum class editor_builtin_prefab_type {
	// GEN INTROSPECTOR enum class editor_builtin_prefab_type
	AQUARIUM
	// END GEN INTROSPECTOR
};

struct editor_prefab_resource_editable {
	// GEN INTROSPECTOR struct editor_prefab_resource_editable
	editor_builtin_prefab_type type = editor_builtin_prefab_type::AQUARIUM;

	editor_prefab_node_editable default_node_properties;
	// END GEN INTROSPECTOR
};

struct editor_prefab_node;
struct editor_prefab_resource {
	using node_type = editor_prefab_node;

	editor_prefab_resource_editable editable;

	std::optional<editor_builtin_prefab_type> official_tag;
	mutable std::variant<typed_entity_flavour_id<box_marker>> scene_flavour_id;

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}

	static const char* get_type_name() {
		return "Prefab";
	}
};

