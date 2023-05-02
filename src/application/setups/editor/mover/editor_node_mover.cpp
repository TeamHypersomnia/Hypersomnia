#include "application/setups/editor/editor_setup.hpp"
#include "application/setups/editor/editor_history.h"
#include "game/cosmos/cosmic_functions.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "application/setups/editor/detail/make_command_from_selections.h"
#include "application/setups/editor/editor_setup_for_each_inspected_entity.hpp"
#include "augs/templates/history.hpp"

using input_type = node_mover_input;

static auto is_movable() {
	return [](const auto typed_handle) {
		return typed_handle.has_independent_transform();
	};
}

bool editor_node_mover::is_active(const editor_history& h) const {
	return get_current_op(h) != node_mover_op::NONE;
}

node_mover_op editor_node_mover::get_current_op(const editor_history& h) const {
	if (!active) {
		return node_mover_op::NONE;
	}

	if (!h.has_last_command()) {
		return node_mover_op::NONE;
	}

	const auto& last = h.last_command();

	if (const auto* const cmd = std::get_if<resize_nodes_command>(std::addressof(last))) {
		(void)cmd;
		return node_mover_op::RESIZING;
	}
	else if (const auto* const cmd = std::get_if<move_nodes_command>(std::addressof(last))) {
		if (cmd->rotation_center.has_value()) {
			return node_mover_op::ROTATING;
		}
		else {
			return node_mover_op::TRANSLATING;
		}
	}

	return node_mover_op::NONE;
}

bool editor_node_mover::escape() {
	if (active) {
		active = false;
		return true;
	}

	return false;
}

bool editor_node_mover::start_transforming_selection(
	const input_type in,
	const std::optional<vec2> rotation_center
) {
	auto& s = in.setup;

	s.finish_rectangular_selection();

	auto command = s.make_command_from_selected_entities<move_nodes_command>("", is_movable());

	if (!command.empty()) {
		active = true;

		command.rotation_center = rotation_center;
		s.post_new_command(std::move(command));

		initial_world_cursor_pos = s.get_world_cursor_pos().discard_fract();

		return true;
	}

	return false;
}

resize_target_point get_resize_target_point(node_mover_input in, const editor_view& view, std::optional<vec2> world_cursor_pos = std::nullopt) {
	const auto& s = in.setup;
	const auto& v = view;

	const auto new_target_point = world_cursor_pos.has_value() ? world_cursor_pos.value() : s.get_world_cursor_pos().discard_fract();
	const auto snapped_target_point = v.snapping_enabled ? vec2(v.grid.get_snapping_corner(new_target_point)) : new_target_point;

	std::optional<snapping_grid> used_grid;

	if (v.snapping_enabled) {
		used_grid = v.grid;
	}

	return { new_target_point, snapped_target_point, used_grid };
}

void editor_node_mover::start_resizing_selection(
	const node_mover_input in,
   	const bool both_axes_simultaneously,
	const resize_nodes_command::active_edges custom_edges
) {
	auto& s = in.setup;

	s.finish_rectangular_selection();

	auto command = s.make_command_from_selected_entities<resize_nodes_command>(
		"",
		[](const auto& h) { return h.can_resize(); }
	);

	command.edges = custom_edges;

	if (!command.empty()) {
		active = true;

		const auto rr = get_resize_target_point(in, s.view);

		initial_world_cursor_pos = rr.actual;

		command.target_point = rr;
		command.both_axes_simultaneously = both_axes_simultaneously;

		s.post_new_command(std::move(command));
	}
}

void editor_node_mover::transform_selection(
	const node_mover_input in,
	const std::optional<vec2> rotation_center,
	const transformr one_shot_delta
) {
	auto& s = in.setup;

	s.finish_rectangular_selection();

	auto command = s.make_command_from_selected_entities<move_nodes_command>("", is_movable());

	if (!command.empty()) {
		command.rotation_center = rotation_center;
		command.move_by = one_shot_delta;

		s.post_new_command(std::move(command));
	}
}


bool editor_node_mover::start_moving_selection(const input_type in) {
	return start_transforming_selection(in, std::nullopt);
}

void editor_node_mover::start_rotating_selection(const input_type in) {
	if (const auto aabb = in.setup.find_selection_aabb()) {
		start_transforming_selection(in, aabb->get_center());
	}
}

static auto make_reinvoker(editor_node_mover& m, const input_type in) {
	const bool pos = m.current_mover_pos_delta(in).has_value();
	const bool rot = m.current_mover_rot_delta(in).has_value();

	return augs::scope_guard(
		[pos, rot, in, &m]() {
			if (pos) {
				m.start_moving_selection(in);
			}

			if (rot) {
				m.start_rotating_selection(in);
			}
		}
	);
}

void editor_node_mover::flip_selection(const input_type in, const flip_flags flip) {
	auto& s = in.setup;

	auto reinvoker = make_reinvoker(*this, in);

	s.finish_rectangular_selection();

	auto command = s.make_command_from_selected_entities<flip_nodes_command>("", is_movable());

	if (!command.empty()) {
		command.flip = flip;

		s.post_new_command(std::move(command));
	}
}

void editor_node_mover::reset_rotation(const input_type in) {
	auto& s = in.setup;

	auto reinvoker = make_reinvoker(*this, in);

	s.finish_rectangular_selection();

	auto command = s.make_command_from_selected_entities<move_nodes_command>("", is_movable());

	if (!command.empty()) {
		command.special = special_entity_mover_op::RESET_ROTATION;
		s.post_new_command(std::move(command));
	}
}

#if TODO
void editor_node_mover::align_individually(const input_type in) {

}
#endif

void editor_node_mover::rotate_selection_once_by(const input_type in, const int degrees) {
	if (const auto aabb = in.setup.find_selection_aabb()) {
		auto reinvoker = make_reinvoker(*this, in);

		transform_selection(in, aabb->get_center(), transformr(vec2::zero, degrees));
	}
}

std::optional<vec2> editor_node_mover::current_mover_pos_delta(const input_type in) const {
	auto& s = in.setup;

	if (active) {
		auto& history = s.history;
		auto& last = history.last_command();

		if (auto* const cmd = std::get_if<move_nodes_command>(std::addressof(last))) {
			if (!cmd->rotation_center) {
				return cmd->move_by.pos;
			}
		}
	}

	return std::nullopt;
}

bool editor_node_mover::show_absolute_mover_pos(node_mover_input in) const {
	auto& s = in.setup;

	if (active) {
		auto& history = s.history;
		auto& last = history.last_command();

		if (auto* const cmd = std::get_if<move_nodes_command>(std::addressof(last))) {
			return cmd->show_absolute_mover_pos_in_ui;
		}
	}

	return false;
}

std::optional<float> editor_node_mover::current_mover_rot_delta(const input_type in) const {
	if (active) {
		auto& history = in.setup.history;
		auto& last = history.last_command();

		if (auto* const cmd = std::get_if<move_nodes_command>(std::addressof(last))) {
			if (cmd->rotation_center) {
				return cmd->move_by.rotation;
			}
		}
	}

	return std::nullopt;
}

bool editor_node_mover::do_mousemotion(const input_type in, vec2 world_cursor_pos) {
	if (active) {
		auto& s = in.setup;

		auto& history = s.history;
		auto& last = history.last_command();
		auto& cosm = s.scene.world;
		const auto& v = s.view;

		if (auto* const cmd = std::get_if<resize_nodes_command>(std::addressof(last))) {
			const auto new_target_point = get_resize_target_point(in, s.view, world_cursor_pos.discard_fract());
			cmd->rewrite_change(new_target_point, s.make_command_input());
		}
		else if (auto* const cmd = std::get_if<move_nodes_command>(std::addressof(last))) {
			const auto new_cursor_pos = world_cursor_pos.discard_fract();

			if (cmd->rotation_center) {
				const auto center = *cmd->rotation_center;

				const auto old_vector = initial_world_cursor_pos - center;
				const auto new_vector = new_cursor_pos - center;

				if (old_vector.is_nonzero() && new_vector.is_nonzero()) {
					const auto degrees = old_vector.full_degrees_between(new_vector);
					auto new_delta = transformr(vec2::zero, degrees);

					if (v.snapping_enabled) {
						auto& r = new_delta.rotation;
						r = v.grid.get_snapped(r);
					}

					cmd->unmove_entities(s.make_command_input());
					cmd->rewrite_change(new_delta, s.make_command_input());
				}
			}
			else {
				auto new_delta = new_cursor_pos - initial_world_cursor_pos;

				cmd->unmove_entities(s.make_command_input());

				if (v.snapping_enabled) {
					cmd->reinfer_moved(cosm);

					if (const auto aabb_before_move = in.setup.find_selection_aabb()) {
						const auto current_aabb = *aabb_before_move + vec2(new_delta);
						const auto snapping_delta = v.grid.get_snapping_delta(current_aabb);
						new_delta += snapping_delta;
					}
				}

				cmd->rewrite_change(new_delta, s.make_command_input());
			}
		}
		else {
			active = false;
		}

		return true;
	}

	return false;
}

bool editor_node_mover::do_left_press(const input_type in) {
	if (active) {
		active = false;
		in.setup.rebuild_arena();
		return true;
	}

	return false;
}

