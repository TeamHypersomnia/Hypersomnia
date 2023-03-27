#pragma once
#include "augs/misc/imgui/standard_window_mixin.h"
#include "view/necessary_resources.h"

class editor_setup;

namespace augs {
	class window;
}

struct editor_toolbar_input {
	editor_setup& setup;
	augs::window& window;
	const ad_hoc_in_atlas_map& ad_hoc_atlas;
	const necessary_images_in_atlas_map& necessary_images;
};

struct editor_toolbar_gui : standard_window_mixin<editor_toolbar_gui> {
	using base = standard_window_mixin<editor_toolbar_gui>;
	using base::base;
	using introspect_base = base;

	ImGuiAxis toolbar_axis = ImGuiAxis_X;
	bool is_docked = true;

	void perform(editor_toolbar_input);
};

