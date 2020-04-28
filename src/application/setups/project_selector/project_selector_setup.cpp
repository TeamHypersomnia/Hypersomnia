#include "game/cosmos/logic_step.h"
#include "game/organization/all_messages_includes.h"

#include "view/viewables/viewables_loading_type.h"

#include "application/config_lua_table.h"
#include "application/setups/project_selector/project_selector_setup.h"

#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/byte_file.h"

#include "application/setups/builder/builder_paths.h"
#include "application/gui/pretty_tabs.h"

#include "augs/misc/imgui/imgui_utils.h"

const entropy_accumulator entropy_accumulator::zero;

project_selector_setup::project_selector_setup() {
	augs::create_directories(BUILDER_DIR);

	load_gui_state();
}

project_selector_setup::~project_selector_setup() {
	save_gui_state();
}

void project_selector_setup::customize_for_viewing(config_lua_table& config) const {
	config.window.name = "Arena builder - project selector";
}

bool projects_list_view::perform() {
	using namespace augs::imgui;

	auto buttons_column_size = ImGui::CalcTextSize("New Project  ");

	auto child = scoped_child("Project list view", ImVec2(-buttons_column_size.x, 0));

	do_pretty_tabs(current_tab);

	switch (current_tab) {
		case project_tab_type::MY_PROJECTS:

			break;

		case project_tab_type::OFFICIAL_TEMPLATES:

			break;

		case project_tab_type::COMMUNITY_ARENAS:

			break;

		default:
			break;
	}

	return false;
}

custom_imgui_result project_selector_setup::perform_custom_imgui(const perform_custom_imgui_input in) {
	using namespace augs::imgui;

	(void)in;

	const auto sz = ImGui::GetIO().DisplaySize;
	set_next_window_rect(xywh(0, 0, sz.x, sz.y), ImGuiCond_Always);

	const auto window_flags = 
		ImGuiWindowFlags_NoDecoration
		| ImGuiWindowFlags_NoInputs
		| ImGuiWindowFlags_NoNav
	;

	auto scope = scoped_window("Project selector", nullptr, window_flags);

	gui.projects_view.perform();

	return custom_imgui_result::NONE;
}

void project_selector_setup::load_gui_state() {
	// TODO: Read/write as yaml

	try {
		augs::load_from_bytes(gui, get_project_selector_gui_state_path());
	}
	catch (const augs::file_open_error&) {
		// We don't care if it does not exist
	}
}

void project_selector_setup::save_gui_state() {
	augs::save_as_bytes(gui, get_project_selector_gui_state_path());
}
