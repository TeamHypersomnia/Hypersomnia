#pragma once
#include "application/setups/editor/editor_history.h"

inline bool editor_history::next_command_has_parent() const {
	return std::visit(
		[](const auto& command) {
			return command.common.has_parent;
		},
		next_command()
	);
}

template <class T, class... RedoArgs>
const T& editor_history::execute_new(T&& command, RedoArgs&&... redo_args) {
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

	return editor_history_base::execute_new(
		std::forward<T>(command),
		std::forward<RedoArgs>(redo_args)...
	);
}

template <class... Args>
void editor_history::redo(Args&&... args) {
	while (editor_history_base::redo(std::forward<Args>(args)...)) {
		if (has_next_command() && next_command_has_parent()) {
			continue;
		}

		break;
	}
}

template <class... Args>
void editor_history::undo(Args&&... args) {
	while (editor_history_base::undo(std::forward<Args>(args)...)) {
		if (has_next_command() && next_command_has_parent()) {
			continue;
		}

		break;
	}
}
