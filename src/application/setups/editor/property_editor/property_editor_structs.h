#pragma once
#include <optional>

#include "3rdparty/imgui/imgui.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/filesystem/path_declaration.h"

#include "game/transcendental/entity_id.h"

#include "game/components/shape_polygon_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/sentience_component.h"
#include "augs/misc/constant_size_vector.h"

struct maybe_official_path;

using edited_field_type_id = type_in_list_id<
	type_list<
		augs::trivial_type_marker,
		std::string,
		convex_partitioned_shape::poly_vector_type,
		convex_partitioned_shape::convex_poly,
		maybe_official_path,
		std::vector<rgba>,
		friction_connection_vector
	>
>;

struct property_editor_settings;

struct description_pair {
	std::string of_old;
	std::string of_new;
};

struct property_editor_state {
	std::optional<ImGuiID> last_active;
	std::string old_description = "";
};

struct field_address {
	// GEN INTROSPECTOR struct field_address
	unsigned offset = static_cast<unsigned>(-1);
	unsigned element_index = static_cast<unsigned>(-1);
	edited_field_type_id type_id;
	// END GEN INTROSPECTOR
};

struct property_editor_input {
	const property_editor_settings& settings;
	property_editor_state& state;
};

