#pragma once
#include "augs/window_framework/window.h"
#include "application/setups/client/client_start_input.h"
#include "application/setups/client/client_vars.h"
#include "augs/math/vec2.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "augs/graphics/texture.h"
#include "application/setups/editor/editor_popup.h"
#include "augs/graphics/renderer.h"
#include "augs/graphics/frame_num_type.h"
#include "hypersomnia_version.h"

namespace augs {
	class window;
};

enum class demo_choice_result_type {
	SHOULD_ANALYZE,

	OK,
	FILE_OPEN_ERROR,
	MIGHT_BE_INCOMPATIBLE
};

class start_client_gui_state : public standard_window_mixin<start_client_gui_state> {
public:
	using base = standard_window_mixin<start_client_gui_state>;
	using base::base;

	std::optional<editor_popup> error_popup;

	bool was_shrinked = false;
	bool will_be_upscaled = false;
	bool do_initial_load = true;

	hypersomnia_version demo_version;
	demo_choice_result_type demo_choice_result = demo_choice_result_type::SHOULD_ANALYZE;

	bool allow_start = false;
	bool mouse_has_to_move_off_browse = false;
	std::optional<augs::frame_num_type> avatar_submitted_when;

	bool perform(
		augs::frame_num_type current_frame,
		augs::renderer& renderer,
		augs::graphics::texture& avatar_preview_tex,
		augs::window& window,
		client_start_input&,
		client_vars&,
		const std::vector<std::string>& official_arena_servers
	);
};
