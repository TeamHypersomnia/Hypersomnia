#pragma once
#include "augs/math/vec2.h"
#include "game/components/light_component.h"
#include "game/cosmos/entity_flavour_id.h"

using editor_light_resource_editable = components::light;

struct editor_light_node;
struct editor_light_resource {
	using node_type = editor_light_node;

	editor_light_resource_editable editable;

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
