#pragma once
#include "application/setups/server/server_start_input.h"
#include "augs/math/vec2.h"
#include "augs/misc/imgui/standard_window_mixin.h"

class start_server_gui_state : public standard_window_mixin<start_server_gui_state> {
public:
	using base = standard_window_mixin<start_server_gui_state>;
	using base::base;

	bool perform(
		server_start_input& into
	);
};
