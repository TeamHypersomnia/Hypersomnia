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
#include "application/setups/editor/commands/mode_commands.h"

#include "application/setups/editor/commands/asset_commands.h"
#include "application/setups/editor/commands/flavour_commands.h"

#include "application/setups/editor/editor_history_declaration.h"
#include "application/setups/editor/editor_command_input.h"

struct editor_history : public editor_history_base {
	using introspect_base = editor_history_base;

	using editor_history_base::editor_history_base;

	inline bool next_command_has_parent() const;

	template <class T>
	const T& execute_new(T&& command, editor_command_input);

	void redo(editor_command_input);
	void undo(editor_command_input);
};