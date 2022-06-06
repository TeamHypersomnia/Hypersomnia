#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"

struct debugger_command_input;

struct debugger_player_gui : standard_window_mixin<debugger_player_gui> {
	using base = standard_window_mixin<debugger_player_gui>;
	using base::base;
	using introspect_base = base;

	void perform(debugger_command_input);
};
