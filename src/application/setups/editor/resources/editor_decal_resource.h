#pragma once
#include <optional>
#include <string>
#include <variant>
#include "augs/pad_bytes.h"
#include "game/cosmos/entity_flavour_id.h"
#include "test_scenes/test_scene_flavour_ids.h"

/*
	A decal left on surfaces, e.g. a bullet hole - chosen per material, see editor_material_decal_variants.
	For now only the official ones, made from test_decal_decorations.
*/

struct editor_decal_resource_editable {
	// GEN INTROSPECTOR struct editor_decal_resource_editable
	pad_bytes<1> pad;
	// END GEN INTROSPECTOR

	bool operator==(const editor_decal_resource_editable&) const = default;
};

struct editor_decal_resource {
	editor_decal_resource_editable editable;

	std::optional<std::variant<test_decal_decorations>> official_tag;

	mutable std::variant<typed_entity_flavour_id<decal_decoration>> scene_flavour_id;

	std::string unique_name;
	const auto& get_display_name() const {
		return unique_name;
	}

	static const char* get_type_name() {
		return "Decal";
	}
};
