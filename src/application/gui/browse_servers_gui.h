#pragma once
#include "application/setups/server/server_start_input.h"
#include "augs/math/vec2.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "application/setups/server/server_instance_type.h"
#include "augs/misc/timing/timer.h"

class browse_servers_gui_state : public standard_window_mixin<browse_servers_gui_state> {
public:
	using base = standard_window_mixin<browse_servers_gui_state>;
	using base::base;

	void perform();
};
