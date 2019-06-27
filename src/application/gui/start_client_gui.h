#pragma once
#include "augs/window_framework/window.h"
#include "application/setups/client/client_start_input.h"
#include "application/setups/client/client_vars.h"
#include "augs/math/vec2.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "augs/graphics/texture.h"
#include "application/setups/editor/editor_popup.h"

namespace augs {
	class window;
};

class start_client_gui_state : public standard_window_mixin<start_client_gui_state> {
public:
	using base = standard_window_mixin<start_client_gui_state>;
	using base::base;

	std::optional<editor_popup> error_popup;
	std::optional<augs::graphics::texture> avatar_preview_tex;

	bool was_shrinked = false;
	bool will_be_upscaled = false;
	bool do_initial_load = true;

	bool allow_start = false;

	bool perform(
		augs::window& window,
		client_start_input&,
		client_vars& 
	);
};
