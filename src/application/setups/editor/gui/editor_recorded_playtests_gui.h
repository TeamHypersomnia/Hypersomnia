#pragma once
#include <optional>
#include <string>
#include <vector>
#include "augs/misc/imgui/standard_window_mixin.h"
#include "view/necessary_resources.h"
#include "application/setups/editor/playtest_recording/editor_playtest_recording.h"

class editor_setup;

namespace augs {
	class window;
}

struct editor_recorded_playtests_input {
	editor_setup& setup;
	augs::window& window;
	const necessary_images_in_atlas_map& necessary_images;
};

struct editor_recorded_playtests_gui : standard_window_mixin<editor_recorded_playtests_gui> {
	using base = standard_window_mixin<editor_recorded_playtests_gui>;
	using base::base;
	using introspect_base = base;

	std::vector<editor_recorded_playtest_entry> entries;
	bool rescan_requested = true;

	augs::path_type selected_folder;
	std::optional<augs::path_type> renamed_folder;
	std::string rename_buffer;
	bool rename_focus_once = false;

	void perform(editor_recorded_playtests_input);
};
