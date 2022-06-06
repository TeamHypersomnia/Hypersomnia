#pragma once
#include "augs/templates/history.hpp"
#include "application/setups/debugger/debugger_history.h"

template <class T>
const T& debugger_history::execute_new(T&& command, const debugger_command_input in) {
	if (!in.allow_new_commands()) {
		return command;
	}

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

	command.common.when_happened = in.get_current_step();

	return debugger_history_base::execute_new(
		std::forward<T>(command),
		in
	);
}
