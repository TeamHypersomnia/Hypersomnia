#pragma once
#include <vector>
#include <variant>

#include "augs/templates/history.h"

#include "application/setups/editor/commands/fill_with_test_scene_command.h"
#include "application/setups/editor/commands/change_flavour_property_command.h"
#include "application/setups/editor/commands/change_entity_property_command.h"
#include "application/setups/editor/commands/delete_entities_command.h"
#include "application/setups/editor/commands/change_common_state_command.h"
#include "application/setups/editor/commands/move_entities_command.h"
#include "application/setups/editor/commands/paste_entities_command.h"
#include "application/setups/editor/commands/duplicate_entities_command.h"
#include "application/setups/editor/commands/change_grouping_command.h"

#include "application/setups/editor/commands/asset_commands.h"

#include "application/setups/editor/editor_history_declaration.h"

struct editor_history : public editor_history_base {
	using introspect_base = editor_history_base;

	using editor_history_base::editor_history_base;

	bool next_command_has_parent() const {
		return std::visit(
			[](const auto& command) {
				return command.common.has_parent;
			},
			next_command()
		);
	}

	bool last_command_has_parent() const {
		return std::visit(
			[](const auto& command) {
				return command.common.has_parent;
			},
			last_command()
		);
	}

	template <class T, class... RedoArgs>
	void execute_new(T&& command, RedoArgs&&... redo_args) {
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

		editor_history_base::execute_new(
			std::forward<T>(command),
			std::forward<RedoArgs>(redo_args)...
		);
	}

	template <class... Args>
	void redo(Args&&... args) {
		while (editor_history_base::redo(std::forward<Args>(args)...)) {
			if (has_next_command() && next_command_has_parent()) {
				continue;
			}

			break;
		}
	}

	template <class... Args>
	void undo(Args&&... args) {
		while (editor_history_base::undo(std::forward<Args>(args)...)) {
			if (has_next_command() && next_command_has_parent()) {
				continue;
			}

			break;
		}
	}
};