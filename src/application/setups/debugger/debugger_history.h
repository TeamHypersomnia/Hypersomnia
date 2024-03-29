#pragma once
#include <vector>
#include <variant>

#include "augs/templates/history.h"

#include "application/setups/debugger/commands/fill_with_test_scene_command.h"
#include "application/setups/debugger/commands/change_flavour_property_command.h"
#include "application/setups/debugger/commands/change_entity_property_command.h"
#include "application/setups/debugger/commands/delete_entities_command.h"
#include "application/setups/debugger/commands/change_common_state_command.h"
#include "application/setups/debugger/commands/move_entities_command.h"
#include "application/setups/debugger/commands/paste_entities_command.h"
#include "application/setups/debugger/commands/duplicate_entities_command.h"
#include "application/setups/debugger/commands/change_grouping_command.h"
#include "application/setups/debugger/commands/mode_commands.h"

#include "application/setups/debugger/commands/asset_commands.h"
#include "application/setups/debugger/commands/flavour_commands.h"

#include "application/setups/debugger/debugger_history_declaration.h"
#include "application/setups/debugger/debugger_command_input.h"

struct debugger_history : public debugger_history_base {
	using introspect_base = debugger_history_base;
	using debugger_history_base::debugger_history_base;

	// GEN INTROSPECTOR struct debugger_history
	augs::date_time when_created;
	// END GEN INTROSPECTOR

	template <class T>
	const T& execute_new(T&& command, debugger_command_input);

	void redo(debugger_command_input);
	void undo(debugger_command_input);

	void seek_to_revision(index_type n, debugger_command_input);
};