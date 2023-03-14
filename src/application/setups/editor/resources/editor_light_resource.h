#pragma once
#include "augs/math/vec2.h"
#include "game/components/light_component.h"
#include "game/cosmos/entity_flavour_id.h"
#include "test_scenes/test_scene_flavour_ids.h"

struct editor_light_resource_editable {
	// GEN INTROSPECTOR struct editor_light_resource_editable
	bool dummy = false;
	// END GEN INTROSPECTOR
};

struct editor_light_node;
struct editor_light_resource {
	using node_type = editor_light_node;

	editor_light_resource_editable editable;

	std::optional<std::variant<test_static_lights>> official_tag;
	mutable std::variant<
		typed_entity_flavour_id<static_light>
	> scene_flavour_id;

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}

	static const char* get_type_name() {
		return "Light";
	}
};
