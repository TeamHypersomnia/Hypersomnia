#pragma once
#include "augs/window_framework/window.h"
#include "application/setups/client/client_start_input.h"
#include "application/setups/client/client_vars.h"
#include "augs/math/vec2.h"
#include "augs/misc/imgui/standard_window_mixin.h"

namespace augs {
	class window;
};

class start_client_gui_state : public standard_window_mixin<start_client_gui_state> {
public:
	using base = standard_window_mixin<start_client_gui_state>;
	using base::base;

	bool perform(
		augs::window& window,
		client_start_input&,
		client_vars& 
	);
};
