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
#include "augs/misc/imgui/imgui_control_wrappers.h"

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

template <class E, class... Args>
void do_pretty_selectables(E& active_pane, Args&&... args) {
	using namespace augs::imgui;

	{
		auto style = scoped_style_var(ImGuiStyleVar_FramePadding, []() { auto padding = ImGui::GetStyle().FramePadding; padding.x *= 4; return padding; }());

		{
			static auto labels = []() {
				static augs::enum_array<std::string, E> label_strs;
				augs::enum_array<const char*, E> c_strs;

				augs::for_each_enum_except_bounds([&c_strs](const E s) {
					label_strs[s] = format_enum(s);
					c_strs[s] = label_strs[s].c_str();
				});

				return c_strs;
			}();

			for (std::size_t i = 0; i < labels.size(); ++i) {
				const auto current_enum = static_cast<E>(i);
				const bool selected = current_enum == active_pane;

				if (ImGui::Selectable(labels[current_enum], selected, std::forward<Args>(args)...)) {
					active_pane = current_enum;
				}
			}
		}
	}
}

bool projects_list_view::perform() {
	using namespace augs::imgui;

	auto left_buttons_column_size = ImGui::CalcTextSize("Community arenas  ");
	auto root = scoped_child("Selector main");

	auto perform_arena_list = [&]() {
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
	};

	centered_text("Arena Builder 2.0 - Project Manager");

	{
		auto list_view = scoped_child("Project categories view", ImVec2(left_buttons_column_size.x, 0));

		const auto selectable_size = ImVec2(left_buttons_column_size.x, left_buttons_column_size.y * 2);
		do_pretty_selectables(current_tab, ImGuiSelectableFlags_None, selectable_size);
	}

	ImGui::SameLine();

	{
		//const auto button_size = ImVec2(left_buttons_column_size.x, 0);

		auto actions = scoped_child("Project list view");

		perform_arena_list();
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
		| ImGuiWindowFlags_NoBringToFrontOnFocus
	;

	auto scope = scoped_window("Project selector main", nullptr, window_flags);

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
