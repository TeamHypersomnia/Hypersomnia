#pragma once
#include "application/setups/debugger/commands/debugger_command_structs.h"
#include "application/setups/debugger/property_debugger/property_debugger_structs.h"
#include "augs/misc/imgui/standard_window_mixin.h"

struct debugger_settings;
struct debugger_command_input;

struct debugger_modes_gui : standard_window_mixin<debugger_modes_gui> {
	using base = standard_window_mixin<debugger_modes_gui>;
	using base::base;
	using introspect_base = base;

	mode_entropy_general perform(const debugger_settings&, debugger_command_input);

private:
	property_debugger_state property_debugger_data;
};
