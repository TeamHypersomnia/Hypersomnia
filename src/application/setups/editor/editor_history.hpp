#pragma once
#include "augs/templates/history.hpp"
#include "application/setups/editor/editor_history.h"
#include "application/setups/editor/editor_player.h"

inline bool editor_history::next_command_has_parent() const {
	return std::visit(
		[](const auto& command) {
			return command.common.has_parent;
		},
		next_command()
	);
}

template <class T>
const T& editor_history::execute_new(T&& command, const editor_command_input in) {
	command.common.reset_timestamp();

	if (has_last_command()) {
		const auto should_make_child = std::visit(
			[](auto& cmd) -> bool {
				return is_create_asset_id_command_v<remove_cref<decltype(cmd)>>;
			},
			last_command()
		);

		if (should_make_child) {
			command.common.has_parent = true;
		}
	}

	command.common.when_happened = in.get_player().get_current_step();

	return editor_history_base::execute_new(
		std::forward<T>(command),
		in
	);
}

inline void editor_history::redo(const editor_command_input cmd_in) {
	auto do_redo = [&]() {
		auto& p = cmd_in.get_player();

		/* TODO: find the target revision by querying for parents first. */
		const auto target_revision = get_current_revision() + 1;
		(void)target_revision;

		if (p.has_testing_started() && has_next_command()) {
			std::visit(
				[&](const auto& cmd) {
					using T = remove_cref<decltype(cmd)>;

					const auto& target_step = cmd.common.when_happened;
					const auto current_step = p.get_current_step();
					ensure_leq(current_step, target_step);

					/* 
						if there are no commands mid-way, seek_to with a later step will only adjust the solvable. 
						seek_to will call redo only after leaving the step at which the command was done.
						It will never invoke the commands of the target step.

						if current step is equal to target step, seek to will return right away.
					*/

					p.seek_to(target_step, cmd_in);
				},
				next_command()
			);

			/* So now we'll have to redo once again */
		}

		return editor_history_base::redo(cmd_in);
	};

	while (do_redo()) {
		if (has_next_command() && next_command_has_parent()) {
			continue;
		}

		break;
	}
}

inline void editor_history::undo(const editor_command_input cmd_in) {
	auto do_undo = [&]() {
		auto& p = cmd_in.get_player();
		/* TODO: find the target revision by querying for parents first. */
		/* TODO: once target revision found, clamp it against get_revision_when_started_testing? Actually, just stop looking for parents when it's found... */
		const auto target_revision = get_current_revision() - 1;

		if (p.has_testing_started()) {
			if (get_current_revision() == p.get_revision_when_started_testing()) {
				return false;
			}

			if (has_last_command()) {
				std::visit(
					[&](const auto& cmd) {
						using T = remove_cref<decltype(cmd)>;

						const auto& target_step = cmd.common.when_happened;
						const auto current_step = p.get_current_step();

						ensure_leq(target_step, current_step);

						if (target_step == cmd.common.when_happened) {
							/* Solvable is compatible. Perform plain undo. */
							editor_history_base::undo(cmd_in);
							return;
						}

						p.seek_to(target_step, cmd_in);

						/* After the workspace is adjusted to target step, we still have to redo until we meet the target revision. */

						ensure_leq(get_current_revision(), target_revision);

						while (get_current_revision() < target_revision) {
							/* Perform plain redo. */
							editor_history_base::redo(cmd_in);
						}
					},
					last_command()
				);

				/* seek_to will get there be redoing, so no more undoing necesary */
				return true;
			}

			return false;
		}

		return editor_history_base::undo(cmd_in);
	};

	while (do_undo()) {
		if (has_next_command() && next_command_has_parent()) {
			continue;
		}

		break;
	}
}
