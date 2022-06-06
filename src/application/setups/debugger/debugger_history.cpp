#include "application/setups/debugger/debugger_history.h"
#include "augs/templates/history.hpp"
#include "application/setups/debugger/debugger_player.h"

template <class T>
static bool has_parent(const T& cmd) {
	return std::visit(
		[](const auto& typed_command) {
			return typed_command.common.has_parent;
		},
		cmd
	);
}

void debugger_history::redo(const debugger_command_input cmd_in) {
	const auto current_revision = get_current_revision();
	const auto last_revision = get_last_revision();

	const auto& commands = get_commands();

	auto target_revision = get_current_revision();

	for (auto& i = target_revision; i < last_revision; ++i) {
		if (i == current_revision) {
			continue;
		}

		if (::has_parent(commands[i])) {
			continue;
		}

		break;
	}

	seek_to_revision(target_revision, cmd_in);
}

void debugger_history::undo(const debugger_command_input cmd_in) {
	const auto& commands = get_commands();
	const auto start_revision = get_current_revision();

	auto target_revision = get_current_revision();

	for (auto& i = target_revision; i > -1; --i) {
		if (i == start_revision) {
			continue;
		}

		if (::has_parent(commands[i])) {
			continue;
		}

		break;
	}

	seek_to_revision(target_revision, cmd_in);
}

void debugger_history::seek_to_revision(
	const index_type target_revision, 
	const debugger_command_input cmd_in
) {
	auto& p = cmd_in.get_player();

	if (target_revision == get_current_revision()) {
		return;
	}

	if (p.has_testing_started() && p.is_recording()) {
		p.begin_replaying(cmd_in.folder);
	}

	auto do_redo = [&]() {
		debugger_history_base::redo(cmd_in);
	};

	auto do_undo = [&]() {
		debugger_history_base::undo(cmd_in);
	};

	auto when_target_cmd_happened = [&]() -> augs::snapshotted_player_step_type {
		if (target_revision == get_first_revision()) {
			return 0;
		}

		const auto& commands = get_commands();

		return std::visit(
			[&](const auto& typed_target_cmd) {
				return typed_target_cmd.common.when_happened;
			},
			commands[target_revision]
		);
	};

	auto undo_until_target = [&]() {
		ensure_less(target_revision, get_current_revision());

		while (target_revision < get_current_revision()) {
			do_undo();
		}
	};

	auto redo_until_target = [&]() {
		ensure_leq(get_current_revision(), target_revision);

		while (get_current_revision() < target_revision) {
			do_redo();
		}
	};

	if (get_current_revision() < target_revision) {
		/* Redoing. */

		if (p.has_testing_started()) {
			const auto target_step = when_target_cmd_happened();
			const auto current_step = p.get_current_step();
			(void)current_step;
			ensure_leq(current_step, target_step);

			/* 
				if there are no commands mid-way, seek_to with a later step will only adjust the solvable. 
				seek_to will call redo only after leaving the step at which the command was done.
				It will never invoke the commands of the target step.

				if current step is equal to target step, seek to will return right away.
			*/

			p.seek_to(target_step, cmd_in);
		}

		/* Once we're at the target solvable step, we still need to invoke the commands that happened at the step. */
		redo_until_target();

		return;
	}

	if (get_current_revision() > target_revision) {
		/* Undoing. */

		if (p.has_testing_started()) {
			const auto target_step = when_target_cmd_happened();
			const auto current_step = p.get_current_step();

			if (current_step == target_step) {
				/* No step happened since the time of the targeted command. */
				undo_until_target();
				return;
			}

			ensure_less(target_step, current_step);

			p.seek_to(target_step, cmd_in);

			redo_until_target();
		}
		else {
			undo_until_target();
		}
	}
}
