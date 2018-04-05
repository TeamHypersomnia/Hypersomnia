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

#include "application/setups/editor/editor_history_declaration.h"

struct editor_history : public editor_history_base {
	// GEN INTROSPECTOR struct editor_history
	// INTROSPECT BASE editor_history_base
	// END GEN INTROSPECTOR

	using editor_history_base::editor_history_base;

	template <class T, class... RedoArgs>
	void execute_new(T&& command, RedoArgs&&... redo_args) {
		command.common.reset_timestamp();

		editor_history_base::execute_new(
			std::forward<T>(command),
			std::forward<RedoArgs>(redo_args)...
		);
	}

private:
	bool next_command_has_parent() const {
		return std::visit(
			[](const auto& command) {
				return command.common.has_parent;
			},
			next_command()
		);
	}

public:
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