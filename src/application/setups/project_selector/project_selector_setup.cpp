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
#include "augs/misc/time_utils.h"
#include "augs/misc/imgui/imgui_game_image.h"

const entropy_accumulator entropy_accumulator::zero;
constexpr auto miniature_size_v = 80;

static augs::path_type get_arenas_directory(const project_tab_type tab_type) {
	switch (tab_type) {
		case project_tab_type::MY_PROJECTS:
			return USER_ARENAS_DIR;

		case project_tab_type::OFFICIAL_TEMPLATES:
			return OFFICIAL_ARENAS_DIR;

		case project_tab_type::COMMUNITY_ARENAS:
			return COMMUNITY_ARENAS_DIR;

		default:
			return "";
	}
}


augs::path_type project_list_entry::get_miniature_path() const {
	return arena_paths(arena_path).miniature_file_path;
}

std::optional<ad_hoc_atlas_subjects> project_selector_setup::get_new_ad_hoc_images() {
	if (rebuild_miniatures) {
		rebuild_miniatures = false;

		ad_hoc_atlas_subjects new_subjects;

		for (const auto& tab : gui.projects_view.tabs) {
			for (const auto& entry : tab.entries) {
				new_subjects.push_back({ entry.miniature_index, entry.get_miniature_path() });
			}
		}

		return new_subjects;
	}

	return std::nullopt;
}

void project_selector_setup::scan_for_all_arenas() {
	auto scan_for = [&](const project_tab_type type) {
		const auto source_directory = get_arenas_directory(type);

		auto register_arena = [&](const auto& arena_folder_path) {
			auto new_entry = project_list_entry();
			new_entry.arena_path = arena_folder_path;

			// TODO: give it a proper timestamp

			new_entry.timestamp = augs::date_time().secs_since_epoch();
			new_entry.miniature_index = miniature_index_counter++;

			return callback_result::CONTINUE; 
		};

		augs::for_each_in_directory(source_directory, register_arena, [](auto&&...) { return callback_result::CONTINUE; });
	};

	scan_for(project_tab_type::MY_PROJECTS);
	scan_for(project_tab_type::OFFICIAL_TEMPLATES);
	scan_for(project_tab_type::COMMUNITY_ARENAS);

	rebuild_miniatures = true;
}

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

void shift_cursor(const vec2 offset) {
	ImGui::SetCursorPos(ImVec2(vec2(ImGui::GetCursorPos()) + offset));
}

bool projects_list_tab_state::perform_list(
	const ad_hoc_in_atlas_map& ad_hoc_in_atlas,
	std::optional<std::string> timestamp_column_name
) {
	using namespace augs::imgui;

	const auto num_columns = timestamp_column_name != std::nullopt ? 2 : 1;

	int current_col = 0;

	auto do_column = [&](std::string label, std::optional<rgba> col = std::nullopt) {
		const bool is_current = current_col == sort_by_column;

		if (is_current) {
			label += ascending ? "▲" : "▼";
		}

		const auto& style = ImGui::GetStyle();

		rgba final_color = is_current ? style.Colors[ImGuiCol_Text] : style.Colors[ImGuiCol_TextDisabled];

		if (col != std::nullopt) {
			final_color = *col;
		}

		auto col_scope = scoped_style_color(ImGuiCol_Text, final_color);

		if (ImGui::Selectable(label.c_str())) {
			if (is_current) {
				ascending = !ascending;
			}
			else 
			{
				sort_by_column = current_col;
				ascending = true;
			}
		}

		++current_col;

		if (num_columns > 1) {
			ImGui::NextColumn();
		}
	};

	ImGui::Columns(num_columns);

	do_column("Name");

	if (timestamp_column_name != std::nullopt) {
		do_column(*timestamp_column_name);
	}

	const auto line_h = ImGui::GetTextLineHeight();

	for (const auto& entry : entries) {
		const auto& path = entry.arena_path;
		const auto arena_name = path.filename();

		const auto selectable_size = ImVec2(0, 1 + miniature_size_v);
		auto id = scoped_id(entry.miniature_index);

		const bool is_selected = path == selected_arena_path;

		const auto local_pos = ImGui::GetCursorPos();

		if (ImGui::Selectable("##Entry", is_selected, ImGuiSelectableFlags_None, selectable_size)) {
			selected_arena_path = path;
		}

		ImGui::SetCursorPos(local_pos);

		ImGui::SameLine();

		const auto image_padding = vec2(0, 4);
		const auto miniature_size = vec2::square(miniature_size_v);
		const auto miniature_entry = mapped_or_nullptr(ad_hoc_in_atlas, entry.miniature_index);

		if (miniature_entry != nullptr) {
			game_image(*miniature_entry, miniature_size, white, image_padding);
		}

		invisible_button("", miniature_size + image_padding);

		const auto x = ImGui::GetCursorPosX();

		text(arena_name);

		const auto prev_y = ImGui::GetCursorPosY() + line_h;
		ImGui::SetCursorPosY(prev_y);
		ImGui::SetCursorPosX(x);

		text_disabled("Arena description");

		ImGui::NextColumn();

		const auto secs_ago = augs::date_time::secs_since_epoch() - entry.timestamp;
		text_disabled(augs::date_time::format_how_long_ago(true, secs_ago));
	}

	return false;
}

std::optional<projects_list_result> projects_list_view::perform(const perform_custom_imgui_input in) {
	using namespace augs::imgui;

	auto left_buttons_column_size = ImGui::CalcTextSize("Community arenas  ");
	auto root = scoped_child("Selector main");

	(void)left_buttons_column_size;

	auto perform_arena_list = [&]() {
		auto& tab = tabs[current_tab];
		auto& ad_hoc = in.ad_hoc_in_atlas;

		switch (current_tab) {
			case project_tab_type::MY_PROJECTS:
				return tab.perform_list(ad_hoc, "Last modified");

			case project_tab_type::OFFICIAL_TEMPLATES:
				return tab.perform_list(ad_hoc, std::nullopt);

			case project_tab_type::COMMUNITY_ARENAS:
				return tab.perform_list(ad_hoc, "When downloaded");

			default:
				return false;
		}
	};

	centered_text("Arena Builder 2.0 - Project Manager");

	{
		const auto text_h = ImGui::GetTextLineHeight();
		const auto create_button_size = ImVec2(0, text_h * 3);

		shift_cursor(vec2(0, text_h / 2));

		const auto before_pos = ImGui::GetCursorPos();

		{
			auto darken_selectables = []() {
				return std::make_tuple(
					scoped_style_color(ImGuiCol_HeaderHovered, rgba(0, 50, 0, 255)),
					scoped_style_color(ImGuiCol_HeaderActive, rgba(0, 100, 0, 255)),
					scoped_style_color(ImGuiCol_Header, rgba(10, 30, 10, 255))
				);
			};


			auto darkened = darken_selectables();

			if (ImGui::Selectable("##CreateNew", true, ImGuiSelectableFlags_None, create_button_size)) {
				current_tab = project_tab_type::MY_PROJECTS;
			}
		}

		const auto after_pos = ImGui::GetCursorPos();

		ImGui::SetCursorPos(before_pos);

		const auto& create_icon = in.necessary_images[assets::necessary_image_id::EDITOR_ICON_CREATE];
		const auto create_icon_size = create_icon.get_original_size();
		const auto icon_padding = vec2(create_icon_size);/// 1.5f;

		const auto bright_green = rgba(100, 255, 100, 255);

		const auto image_offset = vec2(icon_padding.x, create_button_size.y / 2 - create_icon_size.y / 2);
		game_image(create_icon, create_icon_size, bright_green, image_offset);

		const auto text_pos = vec2(before_pos) + image_offset + vec2(create_icon_size.x + icon_padding.x, create_icon_size.y / 2 - text_h / 2);
		ImGui::SetCursorPos(ImVec2(text_pos));
		text_color("CREATE NEW ARENA", bright_green);

		ImGui::SetCursorPos(after_pos);
		shift_cursor(vec2(0, text_h / 2));
	}

	{
		//auto list_view = scoped_child("Project categories view", ImVec2(left_buttons_column_size.x, 0));

		//const auto selectable_size = ImVec2(left_buttons_column_size.x, left_buttons_column_size.y * 2);
	}

	{
		do_pretty_tabs(current_tab);
		//const auto button_size = ImVec2(left_buttons_column_size.x, 0);

		auto actions = scoped_child("Project list view");

		if (const bool choice_performed = perform_arena_list()) {
			auto& tab = tabs[current_tab];

			const bool can_open_directly = current_tab == project_tab_type::MY_PROJECTS;
			const bool needs_to_clone_before_open = !can_open_directly;

			const auto& source_project_path = tab.selected_arena_path;

			if (can_open_directly) {
				const auto& target_project_path = source_project_path;

				(void)target_project_path;

			}
			else if (needs_to_clone_before_open) {

			}
		}
	}

	return std::nullopt;
}

custom_imgui_result project_selector_setup::perform_custom_imgui(const perform_custom_imgui_input in) {
	using namespace augs::imgui;

	const auto sz = ImGui::GetIO().DisplaySize;
	set_next_window_rect(xywh(0, 0, sz.x, sz.y), ImGuiCond_Always);

	const auto window_flags = 
		ImGuiWindowFlags_NoDecoration
		| ImGuiWindowFlags_NoInputs
		| ImGuiWindowFlags_NoNav
		| ImGuiWindowFlags_NoBringToFrontOnFocus
	;

	auto scope = scoped_window("Project selector main", nullptr, window_flags);

	gui.projects_view.perform(in);

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
