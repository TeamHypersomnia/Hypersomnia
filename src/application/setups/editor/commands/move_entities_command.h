#pragma once
#include "augs/enums/callback_result.h"
#include "augs/templates/traits/component_traits.h"

#include "augs/math/transform.h"

#include "game/transcendental/entity_id.h"
#include "game/organization/all_components_declaration.h"
#include "game/transcendental/per_entity_type.h"
#include "game/transcendental/entity_handle_declaration.h"

#include "application/setups/editor/commands/editor_command_structs.h"

struct editor_command_input;

namespace augs {
	struct introspection_access;
}

template <class T>
using typed_entity_id_vector = std::vector<typed_entity_id<T>>;

class move_entities_command {
	friend augs::introspection_access;

	void move_entities(cosmos& cosm);

public:
	using moved_entities_type = per_entity_type_container<typed_entity_id_vector>;
	using delta_type = transformr;

	// GEN INTROSPECTOR class move_entities_command
	editor_command_common common;

private:
	moved_entities_type moved_entities;
	std::vector<std::byte> values_before_change;
public:

	delta_type move_by;
	std::optional<vec2> rotation_center;

	std::string built_description;
	// END GEN INTROSPECTOR

	std::string describe() const;

	void push_entry(const_entity_handle);

	auto size() const {
		return moved_entities.size();
	}

	bool empty() const {
		return size() == 0;
	}

	void rewrite_change(
		const delta_type& new_value,
		const editor_command_input in
	);
	
	void unmove_entities(cosmos& cosm);
	void reinfer_moved(cosmos& cosm);

	void redo(const editor_command_input in);
	void undo(const editor_command_input in);
};

struct active_edges {
	// GEN INTROSPECTOR struct active_edges
	bool top = false;
	bool right = false;
	bool bottom = false;
	bool left = false;
	// END GEN INTROSPECTOR

	active_edges() = default;
	active_edges(transformr, vec2 rect_size, vec2 reference_point, bool both_axes);

	bool operator==(const active_edges b) const {
		return left == b.left && top == b.top && right == b.right && bottom == b.bottom;
	}
};

struct resizing_reference_point {
	// GEN INTROSPECTOR struct resizing_reference_point
	vec2 actual;
	vec2 snapped;
	// END GEN INTROSPECTOR
};

class resize_entities_command {
	friend augs::introspection_access;

	void unresize_entities(cosmos& cosm);

	void resize_entities(cosmos& cosm);
	void reinfer_resized(cosmos& cosm);

public:
	using resized_entities_type = per_entity_type_container<typed_entity_id_vector>;
	using point_type = resizing_reference_point;

	// GEN INTROSPECTOR class resize_entities_command
	editor_command_common common;

private:
	resized_entities_type resized_entities;
	std::vector<std::byte> values_before_change;
	active_edges edges;
public:

	point_type reference_point;
	bool both_axes_simultaneously = false;

	std::string built_description;
	// END GEN INTROSPECTOR

	std::string describe() const;

	void push_entry(const_entity_handle);

	auto size() const {
		return resized_entities.size();
	}

	bool empty() const {
		return size() == 0;
	}

	void rewrite_change(
		const point_type& new_reference_point,
		const editor_command_input in
	);
	
	void redo(const editor_command_input in);
	void undo(const editor_command_input in);

	auto get_active_edges() const {
		return edges;
	}
};
