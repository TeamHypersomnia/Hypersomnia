#pragma once
#include "augs/math/vec2.h"
#include "game/cosmos/entity_flavour_id.h"
#include "test_scenes/test_scene_flavour_ids.h"
#include "game/components/wandering_pixels_component.h"

using ad_hoc_entry_id = uint32_t;

struct editor_wandering_pixels_resource_editable {
	// GEN INTROSPECTOR struct editor_wandering_pixels_resource_editable
	components::wandering_pixels node_defaults;
	vec2i default_size;
	// END GEN INTROSPECTOR
};

struct editor_wandering_pixels_node;
struct editor_wandering_pixels_resource {
	using node_type = editor_wandering_pixels_node;

	editor_wandering_pixels_resource_editable editable;

	std::optional<std::variant<test_wandering_pixels_decorations>> official_tag;

	mutable std::variant<typed_entity_flavour_id<wandering_pixels_decoration>> scene_flavour_id;

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}

	static const char* get_type_name() {
		return "Wandering pixels";
	}
};
