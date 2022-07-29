#pragma once
#include "augs/enums/callback_result.h"
#include "augs/templates/traits/component_traits.h"

#include "augs/math/transform.h"

#include "game/cosmos/entity_id.h"
#include "game/organization/all_components_declaration.h"
#include "game/cosmos/per_entity_type.h"
#include "game/cosmos/entity_handle_declaration.h"

#include "augs/drawing/flip.h"
#include "augs/math/snapping_grid.h"
#include "application/setups/editor/commands/editor_command_meta.h"

struct editor_command_input;

namespace augs {
	struct introspection_access;
}

template <class T>
using typed_entity_id_vector = std::vector<typed_entity_id<T>>;

enum class special_entity_mover_op {
	NONE,
	RESET_ROTATION,
	SNAP_INDIVIDUALLY
};

class move_nodes_command {
	friend augs::introspection_access;

	void move_entities(cosmos& cosm);

public:
	using moved_entities_type = per_entity_type_container<typed_entity_id_vector>;
	using delta_type = transformr;

	editor_command_meta meta;

private:
	moved_entities_type moved_entities;
	std::vector<std::byte> values_before_change;
public:

	special_entity_mover_op special = special_entity_mover_op::NONE;
	delta_type move_by;
	std::optional<vec2> rotation_center;
	bool show_absolute_mover_pos_in_ui = false;

	std::string built_description;

	std::string describe() const;

	void push_entry(const_entity_handle);
	void clear_entries();

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
	
	void unmove_entities(const editor_command_input in);
	void reinfer_moved(cosmos& cosm);

	void redo(const editor_command_input in);
	void undo(const editor_command_input in);

	void clear_undo_state();
	void reselect_moved_entities(const editor_command_input in);
};

class flip_nodes_command {
	friend augs::introspection_access;

	void flip_entities(cosmos& cosm);
	void unmove_entities(cosmos& cosm);

public:
	using flipped_entities_type = per_entity_type_container<typed_entity_id_vector>;
	using delta_type = transformr;

	editor_command_meta meta;
private:
	flipped_entities_type flipped_entities;
	std::vector<std::byte> values_before_change;
public:
	flip_flags flip;
	std::string built_description;

	std::string describe() const;

	void push_entry(const_entity_handle);

	auto size() const {
		return flipped_entities.size();
	}

	bool empty() const {
		return size() == 0;
	}

	void redo(const editor_command_input in);
	void undo(const editor_command_input in);

	void clear_undo_state();

	void reselect_flipped_entities(const editor_command_input in);
};

struct resize_target_point {
	vec2 actual;
	vec2 snapped;
	std::optional<snapping_grid> used_grid;
};

class resize_nodes_command {
	friend augs::introspection_access;

	void unresize_entities(cosmos& cosm);

	void resize_entities(cosmos& cosm);
	void reinfer_resized(cosmos& cosm);

public:
	using resized_entities_type = per_entity_type_container<typed_entity_id_vector>;
	using point_type = resize_target_point;

	struct active_edges {
		bool top = false;
		bool right = false;
		bool bottom = false;
		bool left = false;

		active_edges() = default;
		active_edges(transformr, vec2 rect_size, vec2 target_point, bool both_axes);

		bool operator==(const active_edges b) const {
			return left == b.left && top == b.top && right == b.right && bottom == b.bottom;
		}
	};

	editor_command_meta meta;

private:
	resized_entities_type resized_entities;
	std::vector<std::byte> values_before_change;
	active_edges edges;
public:

	point_type target_point;
	bool both_axes_simultaneously = false;

	std::string built_description;

	std::string describe() const;

	void push_entry(const_entity_handle);

	auto size() const {
		return resized_entities.size();
	}

	bool empty() const {
		return size() == 0;
	}

	void rewrite_change(
		const point_type& new_target_point,
		const editor_command_input in
	);
	
	void redo(const editor_command_input in);
	void undo(const editor_command_input in);

	auto get_active_edges() const {
		return edges;
	}

	void clear_undo_state();
	void reselect_resized_entities(const editor_command_input in);
};
