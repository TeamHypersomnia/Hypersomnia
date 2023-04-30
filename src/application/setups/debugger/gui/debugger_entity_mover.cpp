#include "application/setups/debugger/debugger_setup.hpp"
#include "application/setups/debugger/debugger_history.hpp"
#include "game/cosmos/cosmic_functions.h"
#include "game/detail/inventory/inventory_slot_handle.h"

using input_type = entity_mover_input;

bool debugger_entity_mover::is_active(const debugger_history& h) const {
	return get_current_op(h) != mover_op_type::NONE;
}

mover_op_type debugger_entity_mover::get_current_op(const debugger_history& h) const {
	if (!active) {
		return mover_op_type::NONE;
	}

	if (!h.has_last_command()) {
		return mover_op_type::NONE;
	}

	const auto& last = h.last_command();

	if (const auto* const cmd = std::get_if<resize_entities_command>(std::addressof(last))) {
		(void)cmd;
		return mover_op_type::RESIZING;
	}
	else if (const auto* const cmd = std::get_if<move_entities_command>(std::addressof(last))) {
		if (cmd->rotation_center) {
			return mover_op_type::ROTATING;
		}
		else {
			return mover_op_type::TRANSLATING;
		}
	}

	return mover_op_type::NONE;
}

bool debugger_entity_mover::escape() {
	if (active) {
		active = false;
		return true;
	}

	return false;
}

void debugger_entity_mover::start_transforming_selection(
	const input_type in,
	const std::optional<vec2> rotation_center
) {
	auto& s = in.setup;

	if (s.anything_opened()) {
		s.finish_rectangular_selection();

		auto command = s.make_command_from_selections<move_entities_command>(
			"",
			[](const auto typed_handle) {
				return typed_handle.has_independent_transform();
			}	
		);

		if (!command.empty()) {
			active = true;

			command.rotation_center = rotation_center;
			post_debugger_command(s.make_command_input(), std::move(command));

			initial_world_cursor_pos = s.find_world_cursor_pos().value().discard_fract();
		}
	}
}

resizing_reference_point get_reference_point(entity_mover_input in, std::optional<vec2> world_cursor_pos = std::nullopt) {
	const auto& s = in.setup;
	const auto& v = s.view();

	const auto new_reference_point = world_cursor_pos.has_value() ? world_cursor_pos.value() : s.find_world_cursor_pos().value().discard_fract();
	const auto snapped_reference_point = v.snapping_enabled ? vec2(v.grid.get_snapping_corner(new_reference_point)) : new_reference_point;

	std::optional<snapping_grid> used_grid;

	if (v.snapping_enabled) {
		used_grid = v.grid;
	}

	return { new_reference_point, snapped_reference_point, used_grid };
}

void debugger_entity_mover::start_resizing_selection(
	const entity_mover_input in,
   	const bool both_axes_simultaneously
) {
	auto& s = in.setup;

	if (s.anything_opened()) {
		s.finish_rectangular_selection();

		auto command = s.make_command_from_selections<resize_entities_command>(
			"",
			[](const auto typed_handle) {
				return typed_handle.template has<components::overridden_geo>() && typed_handle.has_independent_transform();
			}	
		);

		if (!command.empty()) {
			active = true;

			const auto rr = get_reference_point(in);

			initial_world_cursor_pos = rr.actual;

			command.reference_point = rr;
			command.both_axes_simultaneously = both_axes_simultaneously;

			post_debugger_command(s.make_command_input(), std::move(command));
		}
	}
}

void debugger_entity_mover::transform_selection(
	const entity_mover_input in,
	const std::optional<vec2> rotation_center,
	const std::optional<transformr> one_shot_delta
) {
	auto& s = in.setup;

	if (s.anything_opened()) {
		s.finish_rectangular_selection();

		auto command = s.make_command_from_selections<move_entities_command>(
			"",
			[](const auto typed_handle) {
				return typed_handle.has_independent_transform();
			}	
		);

		if (!command.empty()) {
			command.rotation_center = rotation_center;
			command.move_by = *one_shot_delta;

			post_debugger_command(s.make_command_input(), std::move(command));
		}
	}
}


void debugger_entity_mover::start_moving_selection(const input_type in) {
	start_transforming_selection(in, std::nullopt);
}

void debugger_entity_mover::start_rotating_selection(const input_type in) {
	if (const auto aabb = in.setup.find_selection_aabb()) {
		start_transforming_selection(in, aabb->get_center());
	}
}

static auto make_reinvoker(debugger_entity_mover& m, const input_type in) {
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

void debugger_entity_mover::flip_selection(const input_type in, const flip_flags flip) {
	auto& s = in.setup;

	if (s.anything_opened()) {
		auto reinvoker = make_reinvoker(*this, in);

		s.finish_rectangular_selection();

		auto command = s.make_command_from_selections<flip_entities_command>(
			"",
			[](const auto typed_handle) {
				return typed_handle.has_independent_transform();
			}	
		);

		if (!command.empty()) {
			command.flip = flip;

			post_debugger_command(s.make_command_input(), std::move(command));
		}
	}
}

void debugger_entity_mover::reset_rotation(const input_type in) {
	auto& s = in.setup;

	if (s.anything_opened()) {
		auto reinvoker = make_reinvoker(*this, in);

		s.finish_rectangular_selection();

		auto command = s.make_command_from_selections<move_entities_command>(
			"",
			[](const auto typed_handle) {
				return typed_handle.has_independent_transform();
			}	
		);

		if (!command.empty()) {
			command.special = special_move_operation::RESET_ROTATION;
			post_debugger_command(s.make_command_input(), std::move(command));
		}
	}
}

#if TODO
void debugger_entity_mover::align_individually(const input_type in) {

}
#endif

void debugger_entity_mover::rotate_selection_once_by(const input_type in, const int degrees) {
	if (const auto aabb = in.setup.find_selection_aabb()) {
		auto reinvoker = make_reinvoker(*this, in);

		transform_selection(in, aabb->get_center(), transformr(vec2::zero, degrees));
	}
}

std::optional<vec2> debugger_entity_mover::current_mover_pos_delta(const input_type in) const {
	auto& s = in.setup;

	if (s.anything_opened() && active) {
		auto& history = s.folder().history;
		auto& last = history.last_command();

		if (auto* const cmd = std::get_if<move_entities_command>(std::addressof(last))) {
			if (!cmd->rotation_center) {
				return cmd->move_by.pos;
			}
		}
	}

	return std::nullopt;
}

std::optional<float> debugger_entity_mover::current_mover_rot_delta(const input_type in) const {
	if (in.setup.anything_opened() && active) {
		auto& history = in.setup.folder().history;
		auto& last = history.last_command();

		if (auto* const cmd = std::get_if<move_entities_command>(std::addressof(last))) {
			if (cmd->rotation_center) {
				return cmd->move_by.rotation;
			}
		}
	}

	return std::nullopt;
}

bool debugger_entity_mover::do_mousemotion(const input_type in, vec2 world_cursor_pos) {
	if (active) {
		auto& s = in.setup;

		auto& history = s.folder().history;
		auto& last = history.last_command();
		auto& cosm = s.work().world;
		const auto& v = s.view();

		if (auto* const cmd = std::get_if<resize_entities_command>(std::addressof(last))) {
			const auto new_reference_point = get_reference_point(in, world_cursor_pos.discard_fract());
			cmd->rewrite_change(new_reference_point, s.make_command_input());
		}
		else if (auto* const cmd = std::get_if<move_entities_command>(std::addressof(last))) {
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

					cmd->unmove_entities(cosm);
					cmd->rewrite_change(new_delta, s.make_command_input());
				}
			}
			else {
				auto new_delta = new_cursor_pos - initial_world_cursor_pos;

				cmd->unmove_entities(cosm);

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

bool debugger_entity_mover::do_left_press(const input_type in) {
	if (active) {
		active = false;
		cosmic::reinfer_all_entities(in.setup.work().world);
		return true;
	}

	return false;
}

