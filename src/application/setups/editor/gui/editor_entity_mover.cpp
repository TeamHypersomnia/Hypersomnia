#include "application/setups/editor/editor_setup.h"

using input_type = entity_mover_input;

bool editor_entity_mover::escape() {
	if (active) {
		active = false;
		return true;
	}

	return false;
}

void editor_entity_mover::transform_selection(
	const input_type in,
	const std::optional<vec2> rotation_center,
	const std::optional<transform> one_shot_delta
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

			if (one_shot_delta) {
				command.move_by = *one_shot_delta;
			}
			else {
				active = true;
				initial_world_cursor_pos = s.get_world_cursor_pos().discard_fract();
			}

			s.folder().history.execute_new(std::move(command), s.make_command_input());
		}
	}
}

void editor_entity_mover::start_moving_selection(const input_type in) {
	transform_selection(in, std::nullopt);
}

void editor_entity_mover::start_rotating_selection(const input_type in) {
	if (const auto aabb = in.setup.find_selection_aabb()) {
		transform_selection(in, aabb->get_center());
	}
}

void editor_entity_mover::rotate_selection_once_by(const input_type in, const int degrees) {
	if (const auto aabb = in.setup.find_selection_aabb()) {
		transform_selection(in, aabb->get_center(), components::transform(vec2::zero, degrees));
	}
}

vec2* editor_entity_mover::current_mover_pos_delta(const input_type in) const {
	auto& s = in.setup;

	if (s.anything_opened() && active) {
		auto& history = s.folder().history;
		auto& last = history.last_command();

		if (auto* const cmd = std::get_if<move_entities_command>(std::addressof(last))) {
			if (!cmd->rotation_center) {
				return std::addressof(cmd->move_by.pos);
			}
		}
	}

	return nullptr;
}

float* editor_entity_mover::current_mover_rot_delta(const input_type in) const {
	if (in.setup.anything_opened() && active) {
		auto& history = in.setup.folder().history;
		auto& last = history.last_command();

		if (auto* const cmd = std::get_if<move_entities_command>(std::addressof(last))) {
			if (cmd->rotation_center) {
				return std::addressof(cmd->move_by.rotation);
			}
		}
	}

	return nullptr;
}

bool editor_entity_mover::do_mousemotion(const input_type in, vec2 world_cursor_pos) {
	if (active) {
		auto& s = in.setup;

		auto& history = s.folder().history;
		auto& last = history.last_command();
		auto& cosm = s.work().world;
		auto& v = s.view();

		if (auto* const cmd = std::get_if<move_entities_command>(std::addressof(last))) {
			const auto new_cursor_pos = world_cursor_pos.discard_fract();

			if (cmd->rotation_center) {
				const auto center = *cmd->rotation_center;

				const auto old_vector = initial_world_cursor_pos - center;
				const auto new_vector = new_cursor_pos - center;

				if (old_vector.non_zero() && new_vector.non_zero()) {
					const auto degrees = old_vector.full_degrees_between(new_vector);
					auto new_delta = components::transform(vec2::zero, degrees);

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

bool editor_entity_mover::do_left_press(const input_type in) {
	if (active) {
		active = false;
		cosmic::reinfer_all_entities(in.setup.work().world);
		return true;
	}

	return false;
}

