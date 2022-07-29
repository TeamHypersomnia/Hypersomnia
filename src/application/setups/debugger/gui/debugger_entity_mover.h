#pragma once
#include <optional>

#include "augs/drawing/flip.h"
#include "augs/math/vec2.h"
#include "augs/math/transform.h"

struct debugger_history;
class debugger_setup;

struct entity_mover_input {
	debugger_setup& setup;
};

enum class mover_op_type {
	NONE,

	TRANSLATING,
	ROTATING,
	RESIZING
};

class debugger_entity_mover {
	bool active = false;
	vec2 initial_world_cursor_pos;

	void transform_selection(
		entity_mover_input in,
		std::optional<vec2> rotation_center,
		std::optional<transformr> one_shot_delta
	);

	void start_transforming_selection(
		entity_mover_input in,
		std::optional<vec2> rotation_center
	);

public:
	bool escape();
	bool is_active(const debugger_history&) const;
	mover_op_type get_current_op(const debugger_history&) const;

	void start_moving_selection(entity_mover_input in);
	void start_rotating_selection(entity_mover_input in);
	void rotate_selection_once_by(entity_mover_input in, int degrees);

	void reset_rotation(entity_mover_input in);
	void flip_selection(entity_mover_input in, flip_flags flip);
	void align_individually(entity_mover_input in);

	void start_resizing_selection(entity_mover_input in, bool both_axes_simultaneously);

	std::optional<vec2> current_mover_pos_delta(entity_mover_input in) const;
	std::optional<float> current_mover_rot_delta(entity_mover_input in) const;

	bool do_mousemotion(entity_mover_input in, vec2 world_cursor_pos);
	bool do_left_press(entity_mover_input in);
};

