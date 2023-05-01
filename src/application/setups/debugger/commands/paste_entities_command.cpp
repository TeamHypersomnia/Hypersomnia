#include "game/cosmos/entity_handle.h"
#include "application/setups/debugger/debugger_command_input.h"
#include "application/setups/debugger/commands/paste_entities_command.h"

std::string paste_entities_command::describe() const {
	return built_description;
}

void paste_entities_command::push_entry(const const_entity_handle handle) {
	(void)handle;
}

bool paste_entities_command::empty() const {
	return size() == 0;
}

void paste_entities_command::redo(const debugger_command_input in) {
	(void)in;
}

void paste_entities_command::undo(const debugger_command_input in) const {
	(void)in;
}
