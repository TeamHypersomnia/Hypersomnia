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

		if (p.has_testing_started() && has_next_command()) {
			std::visit(
				[&](const auto& cmd) {
					using T = remove_cref<decltype(cmd)>;

					if constexpr(needs_valid_solvable_v<T>) {
						const auto& target_step = cmd.common.when_happened;

						p.seek_to(target_step, cmd_in);
					}
				},
				next_command()
			);
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

		if (p.has_testing_started() && has_last_command()) {
			std::visit(
				[&](const auto& cmd) {
					using T = remove_cref<decltype(cmd)>;

					if constexpr(needs_valid_solvable_v<T>) {
						const auto& target_step = cmd.common.when_happened;

						p.seek_to(target_step, cmd_in);
					}
				},
				last_command()
			);
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
