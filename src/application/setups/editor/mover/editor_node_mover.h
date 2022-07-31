#pragma once
#include <optional>

#include "augs/drawing/flip.h"
#include "augs/math/vec2.h"
#include "augs/math/transform.h"

struct editor_history;
class editor_setup;

struct node_mover_input {
	editor_setup& setup;
};

enum class node_mover_op {
	NONE,

	TRANSLATING,
	ROTATING,
	RESIZING
};

class editor_node_mover {
	bool active = false;
	vec2 initial_world_cursor_pos;

	void transform_selection(
		node_mover_input in,
		std::optional<vec2> rotation_center,
		std::optional<transformr> one_shot_delta
	);

	bool start_transforming_selection(
		node_mover_input in,
		std::optional<vec2> rotation_center
	);

public:
	bool escape();
	bool is_active(const editor_history&) const;
	node_mover_op get_current_op(const editor_history&) const;

	bool start_moving_selection(node_mover_input in);
	void start_rotating_selection(node_mover_input in);
	void rotate_selection_once_by(node_mover_input in, int degrees);

	void reset_rotation(node_mover_input in);
	void flip_selection(node_mover_input in, flip_flags flip);
	void align_individually(node_mover_input in);

	void start_resizing_selection(node_mover_input in, bool both_axes_simultaneously);

	std::optional<vec2> current_mover_pos_delta(node_mover_input in) const;
	std::optional<float> current_mover_rot_delta(node_mover_input in) const;
	bool show_absolute_mover_pos(node_mover_input in) const;

	bool do_mousemotion(node_mover_input in, vec2 world_cursor_pos);
	bool do_left_press(node_mover_input in);
};

