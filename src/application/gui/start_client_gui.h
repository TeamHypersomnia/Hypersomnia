#pragma once
#include "augs/window_framework/window.h"
#include "application/setups/client/client_start_input.h"
#include "application/setups/client/client_vars.h"
#include "augs/math/vec2.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "augs/graphics/texture.h"
#include "augs/misc/imgui/simple_popup.h"
#include "augs/graphics/renderer.h"
#include "augs/graphics/frame_num_type.h"
#include "hypersomnia_version.h"
#include "application/setups/client/demo_file.h"

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

	std::optional<simple_popup> error_popup;

	bool was_shrinked = false;
	bool will_be_upscaled = false;
	bool do_initial_load = true;

	demo_file_meta demo_meta;
	demo_choice_result_type demo_choice_result = demo_choice_result_type::SHOULD_ANALYZE;
	std::string custom_address;
	std::string demo_size;

	bool allow_start = false;
	bool mouse_has_to_move_off_browse = false;
	std::optional<augs::frame_num_type> avatar_submitted_when;

	void clear_demo_choice();

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
