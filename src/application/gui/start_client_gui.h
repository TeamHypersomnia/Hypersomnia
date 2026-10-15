#pragma once
#include <optional>
#include <string>
#include <utility>
#include "augs/window_framework/window.h"
#include "application/setups/client/client_connect_string.h"
#include "application/setups/client/client_vars.h"
#include "augs/math/vec2.h"
#include "augs/misc/imgui/standard_window_mixin.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/graphics/texture.h"
#include "augs/misc/imgui/simple_popup.h"
#include "augs/graphics/renderer.h"
#include "augs/graphics/frame_num_type.h"
#include "hypersomnia_version.h"
#include "application/setups/client/demo_file.h"
#include "application/gui/start_client_tab_type.h"

namespace augs {
	class window;
};

enum class demo_choice_result_type {
	SHOULD_ANALYZE,

	OK,
	FILE_OPEN_ERROR,
	MIGHT_BE_INCOMPATIBLE
};

struct server_list_entry;
class favorite_servers;

class start_client_gui_state : public standard_window_mixin<start_client_gui_state> {
public:
	using base = standard_window_mixin<start_client_gui_state>;
	using base::base;

	bool is_steam_client = false;

	demo_file_meta demo_meta;
	demo_choice_result_type demo_choice_result = demo_choice_result_type::SHOULD_ANALYZE;

	augs::path_type demo_path;
	std::string custom_address;

	/*
		Custom server: its address and the key it must have - or the address only, with nothing verifying who answers.
	*/

	bool with_key = true;
	std::string custom_key;

	/*
		Which field had the keyboard last frame - a paste there is taken whole (see perform).
	*/

	bool address_field_active = false;

	/*
		Auto-paste: the clipboard taken last (a clipboard is taken once - the fields the player changed since stay),
		and the address and key it filled in - "Auto-pasted from clipboard" shows while they are as filled.
	*/

	std::string last_autopasted_clipboard;
	std::optional<std::pair<std::string, std::string>> autopasted;

	/*
		When the clipboard is looked at: the window opening, the window or the game getting the focus back (e.g. after
		copying in a browser), the Unlisted server tab chosen.
	*/

	bool was_focused = false;
	uint32_t times_window_activated = 0;
	start_client_tab_type tab_last_frame = start_client_tab_type::BEST_SERVER;
	std::string demo_size;

	bool allow_start = false;

	start_client_tab_type current_tab = start_client_tab_type::BEST_SERVER;
	bool request_refresh_best_server = false;
	bool request_server_list_open = false;

	void clear_demo_choice();

	bool perform(
		const std::optional<server_list_entry>& best_server,
		const bool refresh_in_progress,
		augs::window& window,
		client_connect_string&,
		std::string& into_displayed_connecting_server_name,
		client_vars&,
		const bool streamer_mode,
		favorite_servers& favorites
	);
};
