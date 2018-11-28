#pragma once
#include "application/setups/client/client_start_input.h"
#include "augs/math/vec2.h"
#include "augs/misc/imgui/standard_window_mixin.h"

class start_client_gui_state : public standard_window_mixin<start_client_gui_state> {
public:
	using base = standard_window_mixin<start_client_gui_state>;
	using base::base;

	bool perform(
		client_start_input& into
	);
};
