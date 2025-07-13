#pragma once
#include <cstdint>
#include "augs/math/vec2.h"
#include "game/cosmos/entity_flavour_id.h"
#include "test_scenes/test_scene_flavour_ids.h"

using ad_hoc_entry_id = uint32_t;

struct editor_explosive_resource_editable {
	// GEN INTROSPECTOR struct editor_explosive_resource_editable
	bool dummy = false;
	// END GEN INTROSPECTOR

	bool operator==(const editor_explosive_resource_editable&) const = default;
};

struct editor_explosive_node;
struct editor_explosive_resource {
	using node_type = editor_explosive_node;

	editor_explosive_resource_editable editable;

	ad_hoc_entry_id thumbnail_id = static_cast<ad_hoc_entry_id>(-1);

	std::optional<std::variant<test_hand_explosives>> official_tag;

	mutable std::variant<typed_entity_flavour_id<hand_explosive>> scene_flavour_id;

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}

	static const char* get_type_name() {
		return "Explosive";
	}
};
