#include "augs/log.h"
#include "game/cosmos/logic_step.h"
#include "game/organization/all_messages_includes.h"

#include "view/viewables/viewables_loading_type.h"

#include "application/config_lua_table.h"
#include "application/setups/project_selector/project_selector_setup.h"

#include "augs/readwrite/byte_readwrite.h"
#include "augs/readwrite/byte_file.h"

#include "application/setups/editor/editor_paths.h"
#include "application/gui/pretty_tabs.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/time_utils.h"
#include "augs/misc/imgui/imgui_game_image.h"
#include "application/arena/arena_paths.h"
#include "application/setups/editor/project/editor_project_paths.h"
#include "application/setups/editor/project/editor_project_readwrite.h"
#include "application/setups/editor/project/editor_project.h"
#include "application/setups/editor/detail/maybe_different_colors.h"
#include "augs/readwrite/json_readwrite_errors.h"
#include "augs/string/path_sanitization.h"
#include "augs/filesystem/find_path.h"
#include "augs/misc/pool/pool_allocate.h"

#include "augs/readwrite/json_readwrite.h"
#include "augs/window_framework/window.h"

#include "augs/templates/in_order_of.h"

constexpr auto miniature_size_v = 80;
constexpr auto preview_size_v = 400;

std::string sanitize_arena_short_description(std::string in) {
	int newlines = 0;

	for (auto& c : in) {
		if (c == '\n') {
			++newlines;

			if (newlines > 1) {
				c = ' ';
			}
		}
	}

	return in;
}

static augs::path_type get_arenas_directory(const project_tab_type tab_type) {
	switch (tab_type) {
		case project_tab_type::MY_PROJECTS:
			return EDITOR_PROJECTS_DIR;

		case project_tab_type::OFFICIAL_ARENAS:
			return OFFICIAL_ARENAS_DIR;

		case project_tab_type::DOWNLOADED_ARENAS:
			return DOWNLOADED_ARENAS_DIR;

		default:
			return "";
	}
}

project_selector_setup::project_selector_setup() {
	augs::create_directories(EDITOR_PROJECTS_DIR);

	scan_for_all_arenas();
}

project_selector_setup::~project_selector_setup() {

}

augs::path_type project_list_entry::get_miniature_path() const {
	return editor_project_paths(arena_path).miniature;
}

arena_identifier project_list_entry::get_arena_name() const {
	return arena_name;
}

std::optional<ad_hoc_atlas_subjects> project_selector_setup::get_new_ad_hoc_images() {
	if (rebuild_miniatures) {
		rebuild_miniatures = false;

		ad_hoc_atlas_subjects new_subjects;

		for (const auto& tab : gui.projects_view.tabs) {
			for (const auto& entry : tab.entries) {
				const auto blank_path = "content/gfx/necessary/blank_transparent.png";
				const auto miniature_path = entry.get_miniature_path();

				new_subjects.push_back({ 
					entry.miniature_index, 
					augs::exists(miniature_path) ? miniature_path : blank_path 
				});
			}
		}

		return new_subjects;
	}

	return std::nullopt;
}

static editor_project_about read_about_from(const augs::path_type& arena_folder_path) {
	const auto paths = editor_project_paths(arena_folder_path);

	return editor_project_readwrite::read_only_project_about(paths.project_json);
}

editor_project_meta read_meta_from(const augs::path_type& arena_folder_path) {
	const auto paths = editor_project_paths(arena_folder_path);

	return editor_project_readwrite::read_only_project_meta(paths.project_json);
}

std::optional<project_tab_type> project_selector_setup::is_project_name_taken(const arena_identifier& arena_name) const {
	using P = project_tab_type;

	if (arena_name == arena_identifier("autosave")) {
		return project_tab_type::OFFICIAL_ARENAS;
	}

	for (auto i = P::MY_PROJECTS; i < P::COUNT; i = P(int(i) + 1)) {
		for (auto& e : gui.projects_view.tabs[i].entries) {
			if (arena_name == e.arena_name) {
				return i;
			}
		}
	}

	return std::nullopt;
}

void project_selector_setup::scan_for_all_arenas() {
	miniature_index_counter = 0;

	auto scan_for = [&](const project_tab_type type) {
		auto& view = gui.projects_view;
		view.tabs[type].entries.clear();

		const auto source_directory = get_arenas_directory(type);

		auto register_arena = [&](const auto& arena_folder_path) {
			const auto paths = editor_project_paths(arena_folder_path);

			const auto sanitized = sanitization::sanitize_arena_path(source_directory, paths.arena_name);
			const auto sanitized_path = std::get_if<augs::path_type>(&sanitized);

			if (!sanitized_path || *sanitized_path != arena_folder_path) {
				LOG("%x is not a correct arena path! See if the arena name isn't too long or if it contains forbidden characters.", arena_folder_path);

				return callback_result::CONTINUE;
			}

			auto new_entry = project_list_entry();
			new_entry.arena_name = paths.arena_name;
			new_entry.arena_path = *sanitized_path;

			// TODO: give it a proper timestamp

			new_entry.miniature_index = miniature_index_counter++;
			try {
				new_entry.about = ::read_about_from(*sanitized_path);
				new_entry.meta = ::read_meta_from(*sanitized_path);
			}
			catch (const augs::file_open_error& err) {
				editor_project_about err_abouts;
				err_abouts.short_description = "Missing file: " + paths.project_json.filename().string();
				err_abouts.full_description = err.what();

				new_entry.about = err_abouts;

				if (type == project_tab_type::OFFICIAL_ARENAS) {
					/* For now not having a project json file is used to determine if a map is a legacy one */
					return callback_result::CONTINUE;
				}
			}
			catch (const augs::json_deserialization_error& err) {
				editor_project_about err_abouts;
				err_abouts.short_description = "Parsing error: " + paths.project_json.filename().string();
				err_abouts.full_description = err.what();

				new_entry.about = err_abouts;
			}

			(void)paths;

			auto& view = gui.projects_view;
			view.tabs[type].entries.push_back(new_entry);

			return callback_result::CONTINUE; 
		};

		try {
			augs::for_each_directory_in_directory(source_directory, register_arena);
		}
		catch (const augs::filesystem_error& err) {
			//LOG("No folder: %x", source_directory);
		}
	};

	scan_for(project_tab_type::MY_PROJECTS);
	scan_for(project_tab_type::OFFICIAL_ARENAS);
	scan_for(project_tab_type::DOWNLOADED_ARENAS);

	rebuild_miniatures = true;
}

void project_selector_setup::customize_for_viewing(config_lua_table& config) const {
	config.window.name = "Hypersomnia Editor - Project Selector";
}

bool projects_list_tab_state::perform_list(
	ImGuiTextFilter& filter,
	const ad_hoc_in_atlas_map& ad_hoc_atlas,
	std::optional<std::string> timestamp_column_name,
	augs::window& window
) {
	using namespace augs::imgui;

	const auto num_columns = timestamp_column_name.has_value() ? 2 : 1;

	int current_col = 0;

	auto do_column = [&](std::string label, std::optional<rgba> col = std::nullopt) {
		const bool is_current = current_col == sort_by_column;

		if (is_current) {
			label += ascending ? "▲" : "▼";
		}

		const auto& style = ImGui::GetStyle();

		auto final_color = rgba(is_current ? style.Colors[ImGuiCol_Text] : style.Colors[ImGuiCol_TextDisabled]);

		if (col.has_value()) {
			final_color = *col;
		}

		auto col_scope = scoped_style_color(ImGuiCol_Text, final_color);

		if (ImGui::Selectable(label.c_str())) {
			if (is_current) {
				ascending = !ascending;
			}
			else {
				sort_by_column = current_col;
				ascending = true;
			}
		}

		++current_col;

		if (num_columns > 1) {
			ImGui::NextColumn();
		}
	};

	const auto avail = ImGui::GetContentRegionAvail();

	ImGui::Columns(num_columns);

	if (num_columns > 1) {
		ImGui::SetColumnWidth(0, avail.x * 0.8f);
	}

	do_column("Name");

	if (timestamp_column_name.has_value()) {
		do_column(*timestamp_column_name);
	}

	ImGui::Separator();

	const auto line_h = ImGui::GetTextLineHeight();

	bool anything_chosen = false;

	auto process_entry = [&](const auto& entry) {
		const auto& path = entry.arena_path;
		const auto arena_name = entry.get_arena_name();

		if (filter.IsActive() && !filter.PassFilter(arena_name.c_str())) {
			return;
		}

		const auto selectable_size = ImVec2(0, 1 + miniature_size_v);
		auto id = scoped_id(entry.miniature_index);

		const bool is_selected = path == selected_arena_path;

		const auto local_pos = ImGui::GetCursorPos();

		{
			auto darkened_selectables = scoped_selectable_colors({
				rgba(255, 255, 255, 20),
				rgba(255, 255, 255, 30),
				rgba(255, 255, 255, 60)
			});

			if (ImGui::Selectable("##Entry", is_selected, ImGuiSelectableFlags_SpanAllColumns, selectable_size)) {
			}

			if (ImGui::IsItemClicked()) {
				selected_arena_path = path;
			}

			if (!anything_chosen && ImGui::IsItemClicked()) {
				if (ImGui::IsMouseDoubleClicked(0)) {
					anything_chosen = true;
					selected_arena_path = path;
				}
			}

			if (ImGui::BeginPopupContextItem()) {
				if (ImGui::Selectable("Reveal in explorer")) {
					window.reveal_in_explorer(path);
				}

				ImGui::EndPopup();
			}
		}

		const auto after_pos = ImGui::GetCursorPos();

		ImGui::SetCursorPos(local_pos);

		ImGui::SameLine();

		const auto image_padding = vec2(0, 0);
		const auto miniature_entry = mapped_or_nullptr(ad_hoc_atlas, entry.miniature_index);
		const auto target_miniature_size = vec2::square(miniature_size_v);

		if (miniature_entry != nullptr) {
			const auto miniature_size = miniature_entry->get_original_size();
			const auto resized_miniature_size = vec2i::scaled_to_max_size(miniature_size, miniature_size_v);

			const auto offset = (target_miniature_size - resized_miniature_size) / 2;
			game_image(*miniature_entry, resized_miniature_size, white, offset + image_padding, augs::imgui_atlas_type::AD_HOC);
		}

		invisible_button("invisible_miniature", target_miniature_size + image_padding);

		ImGui::SameLine();

		const auto x = ImGui::GetCursorPosX();
		const auto prev_y = ImGui::GetCursorPosY() + line_h;

		text(arena_name);

		ImGui::SetCursorPosY(prev_y);
		ImGui::SetCursorPosX(x);

		text_disabled(sanitize_arena_short_description(entry.about.short_description));

		ImGui::SetCursorPos(after_pos);

		if (num_columns > 1) {
			ImGui::NextColumn();

			const auto date = augs::date_time::from_utc_timestamp(entry.meta.version_timestamp);

			const auto ago = typesafe_sprintf(
				"%x\n%x", 
				date.get_readable_day_hour(),
				date.how_long_ago_tell_seconds()
			);

			text_disabled(ago);

			ImGui::NextColumn();
		}
	};

	if (sort_by_column == 0) {
		::in_order_of(
			entries,
			[&](const auto& entry) {
				return entry.get_arena_name();
			},
			[&](const auto& a, const auto& b) {
				return ascending ? augs::natural_order(a, b) : augs::natural_order(b, a);
			},
			process_entry
		);
	}
	else {
		::in_order_of(
			entries,
			[&](const auto& entry) {
				return entry.meta.version_timestamp;
			},
			[&](const auto& a, const auto& b) {
				return ascending ? a < b : a > b;
			},
			process_entry
		);
	}

	return anything_chosen;
}


project_list_entry* projects_list_tab_state::find_selected() {
	if (selected_arena_path.empty()) {
		return nullptr;
	}

	for (auto& e : entries) {
		if (e.arena_path == selected_arena_path) {
			return std::addressof(e);
		}
	}

	return nullptr;
}

project_list_view_result projects_list_view::perform(const perform_custom_imgui_input in) {
	using namespace augs::imgui;
	auto result = project_list_view_result::NONE;

	auto left_buttons_column_size = ImGui::CalcTextSize("Downloaded arenas  ");
	auto root = scoped_child("Selector main", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	(void)left_buttons_column_size;

	thread_local ImGuiTextFilter filter;

	auto perform_arena_list = [&]() {
		auto& tab = tabs[current_tab];
		auto& ad_hoc = in.ad_hoc_atlas;

		switch (current_tab) {
			case project_tab_type::MY_PROJECTS:
			return tab.perform_list(filter, ad_hoc, "Last updated", in.window);

			case project_tab_type::OFFICIAL_ARENAS:
			return tab.perform_list(filter, ad_hoc, "Last updated", in.window);

			case project_tab_type::DOWNLOADED_ARENAS:
			return tab.perform_list(filter, ad_hoc, "Last updated", in.window);

			default:
			return false;
		}
	};

	centered_text("Hypersomnia Editor - Project Selector");

	const auto button_size_mult = 3.0f;
	const auto button_padding_mult = 0.5f;

	const bool create_pressed = selectable_with_icon(
		in.necessary_images[assets::necessary_image_id::EDITOR_ICON_FILE],
		"CREATE NEW ARENA",
		button_size_mult,
		vec2(1.0f, button_padding_mult),
		rgba(100, 255, 100, 255),
		{
			rgba(10, 50, 10, 255),
			rgba(20, 70, 20, 255),
			rgba(20, 90, 20, 255)
		}
	);

	if (create_pressed) {
		result = project_list_view_result::OPEN_CREATE_DIALOG;
	}

	{
		do_pretty_tabs(current_tab);

		const auto avail = ImGui::GetContentRegionAvail();
		const auto proj_list_width = avail.x * 0.6f;
		const auto proj_desc_width = avail.x * 0.4f;

		const auto text_h = ImGui::GetTextLineHeight();
		const auto space_for_clone_button = text_h * (button_padding_mult * 2 + button_size_mult);

		filter.Draw();

		{
			auto scope = scoped_child("Project list view", ImVec2(proj_list_width, -space_for_clone_button));

			if (const bool choice_performed = perform_arena_list()) {
				const bool can_open_directly = current_tab == project_tab_type::MY_PROJECTS;
				const bool needs_to_clone_before_open = !can_open_directly;

				if (can_open_directly) {
					return project_list_view_result::OPEN_SELECTED_PROJECT;

				}
				else if (needs_to_clone_before_open) {
					return project_list_view_result::OPEN_CREATE_FROM_SELECTED_DIALOG;
				}
			}
		}

		ImGui::SameLine();

		auto& tab = tabs[current_tab];
		const auto selected_entry = tab.find_selected();

		{
			auto fix_background_color = scoped_style_color(ImGuiCol_ChildBg, ImVec4{0.0f, 0.0f, 0.0f, 0.0f});

			auto scope = scoped_child("Project description view", ImVec2(proj_desc_width, -space_for_clone_button), false);

			if (selected_entry != nullptr) {
				auto& entry = *selected_entry;
				auto& about = entry.about;

				const auto image_padding = vec2(5, 5);
				const auto image_internal_padding = vec2i(15, 15);
				const auto preview_entry = mapped_or_nullptr(in.ad_hoc_atlas, entry.miniature_index);
				const auto target_preview_size = vec2::square(preview_size_v);

				if (preview_entry != nullptr) {
					const auto preview_size = preview_entry->get_original_size();
					const auto resized_preview_size = vec2i(vec2(preview_size) * (static_cast<float>(preview_size_v) / preview_size.bigger_side()));

					const auto offset = (target_preview_size - resized_preview_size) / 2;

					auto bg_size = target_preview_size + image_internal_padding * 2;
					bg_size.x = ImGui::GetContentRegionAvail().x;

					game_image(in.necessary_images[assets::necessary_image_id::BLANK], bg_size, rgba(0, 0, 0, 100), image_padding, augs::imgui_atlas_type::GAME);
					game_image(*preview_entry, resized_preview_size, white, offset + image_padding + image_internal_padding, augs::imgui_atlas_type::AD_HOC);

					invisible_button("invisible_preview", target_preview_size + image_padding + image_internal_padding * 2);

					auto fix_background_color = scoped_style_color(ImGuiCol_ChildBg, ImVec4{0.0f, 0.0f, 0.0f, 0.0f});

					shift_cursor(image_padding);

					auto scope = scoped_child("descview", ImVec2(0, 0), true);

					text(std::string(entry.get_arena_name()) + "\n\n");

					ImGui::Columns(2);

					float max_w = 0;

					for (const auto& r : about.credits) {
						max_w = std::max(max_w, ImGui::CalcTextSize(r.role.c_str()).x);
					}

					ImGui::SetColumnWidth(0, max_w + text_h);

					for (const auto& r : about.credits) {
						text_disabled(std::string(r.role) + ": ");
						ImGui::NextColumn();

						text(r.person);
						ImGui::NextColumn();
					}

					ImGui::Columns(1);

					text("\n");

					ImGui::PushTextWrapPos();

					text_color(about.full_description, rgba(210, 210, 210, 255));

					ImGui::PopTextWrapPos();
				}
			}
			else {
				text_disabled("(No project selected)");
			}
		}

		if (selected_entry != nullptr) {
			auto do_big_button = [&](auto label, auto icon, auto result_if) {
				if (selectable_with_icon(
					in.necessary_images[icon],
					label,
					button_size_mult,
					vec2(1.0f, button_padding_mult),
					rgba(120, 220, 255, 255),
					{
						rgba(15, 40, 70, 255),
						rgba(35, 60, 90, 255),
						rgba(55, 80, 110, 255)
					}
				)) {
					result = result_if;
				}
			};

			if (current_tab == project_tab_type::MY_PROJECTS) {
				ImGui::Columns(2);
				do_big_button("OPEN SELECTED", assets::necessary_image_id::EDITOR_ICON_OPEN, project_list_view_result::OPEN_SELECTED_PROJECT);
				ImGui::NextColumn();
				do_big_button("CLONE FROM SELECTED", assets::necessary_image_id::EDITOR_ICON_CLONE, project_list_view_result::OPEN_CREATE_FROM_SELECTED_DIALOG);
				ImGui::Columns(1);
			}
			else {
				do_big_button("CREATE FROM SELECTED", assets::necessary_image_id::EDITOR_ICON_CLONE, project_list_view_result::OPEN_CREATE_FROM_SELECTED_DIALOG);
			}
		}
	}

	return result;
}

augs::path_type projects_list_view::get_selected_project_path() const {
	return tabs[current_tab].selected_arena_path;
}

augs::path_type project_selector_setup::get_selected_project_path() const {
	return gui.projects_view.get_selected_project_path();
}

void projects_list_view::select_project(const project_tab_type tab, const augs::path_type& path) {
	current_tab = tab;
	tabs[tab].selected_arena_path = path;
}

project_list_entry* projects_list_view::find_selected() {
	return tabs[current_tab].find_selected();
}

bool create_new_project_gui::perform(const project_selector_setup& setup) {
	using namespace augs::imgui;

	centered_size_mult = vec2(0.6f, 0.6f);

	const auto flags = 
		ImGuiWindowFlags_NoResize | 
		ImGuiWindowFlags_NoMove | 
		ImGuiWindowFlags_NoCollapse | 
		ImGuiWindowFlags_NoSavedSettings
	;

	auto create_window = make_scoped_window(flags);
	//std::optional<sanitization::forbidden_path_type> maybe_error;

	if (!create_window) {
		return false;
	}

	{
		auto child = scoped_child("create view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));

		if (!cloning_from.empty()) {
			text_color(typesafe_sprintf("Cloning from %x", cloning_from), yellow);
		}

		text_color("New arena name", green);

		input_text("##Map name", name);

		text("Be creative!\nIf an arena with the same name exists on somebody's server,\npeople will have to delete it in order to play your arena.");

		const auto map_name_hint = typesafe_sprintf(
			"The arena name must be at most %x characters.\nIt can only contain the following characters:\n%x",
			max_arena_name_length_v,
			sanitization::portable_alphanumeric_set
		);

		text_disabled(map_name_hint);

		text("");

		ImGui::Separator();

		text_color("Short description", green);
		input_multiline_text("##Short description", short_description, 3);
		text("Describe your arena in two sentences.");

		auto sanitization_result = sanitization::sanitize_arena_path(EDITOR_PROJECTS_DIR, std::string(name));
		const auto err = std::get_if<sanitization::forbidden_path_type>(&sanitization_result);

		const auto taken_reason = setup.is_project_name_taken(std::string(name));

		auto describe_reason_taken = [this](const project_tab_type reason) {
			switch (reason) {
				if (name == arena_identifier("autosave")) {
					return "Forbidden name.";
				}

				case project_tab_type::MY_PROJECTS:
					return "Project with this name already exists.";
				case project_tab_type::OFFICIAL_ARENAS:
					return "An official arena with this name already exists.";
				case project_tab_type::DOWNLOADED_ARENAS:
					return "Warning: a downloaded arena with the same name detected.";
				case project_tab_type::COUNT:
				default:
					return "Unknown error.";
			}
		};

		text("\n");

		if (taken_reason.has_value()) {
			const auto reason_str = describe_reason_taken(*taken_reason);

			if (taken_reason == project_tab_type::DOWNLOADED_ARENAS) {
				text_color(reason_str, yellow);
			}
			else {
				text_color(reason_str, red);
			}
		}
		else if (err) {
			text_color(typesafe_sprintf("Cannot create an arena with this name.\n%x", sanitization::describe_for_arena(*err)), red);
		}
	}

	auto sanitization_result = sanitization::sanitize_arena_path(EDITOR_PROJECTS_DIR, std::string(name));
	auto sanitized_path = std::get_if<augs::path_type>(&sanitization_result);
	
	const auto taken_reason = setup.is_project_name_taken(std::string(name));

	const bool is_disabled = 
		name == arena_identifier("autosave")
		|| (taken_reason.has_value() && taken_reason != project_tab_type::DOWNLOADED_ARENAS)
		|| sanitized_path == nullptr 
		|| *sanitized_path != (EDITOR_PROJECTS_DIR / std::string(name))
	;

	{
		auto child = scoped_child("create cancel");

		ImGui::Separator();

		{

			auto scope = maybe_disabled_cols(is_disabled);

			if (ImGui::Button("Create")) {
				return true;
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			close();
		}
	}

	return false;
}

bool project_selector_setup::handle_input_before_imgui(
	handle_input_before_imgui_input in
) {
	using namespace augs::event;

	if (in.e.msg == message::activate || in.e.msg == message::click_activate) {
		scan_for_all_arenas();
	}

	return false;
}

bool project_selector_setup::handle_input_before_game(
	const handle_input_before_game_input
) {
	return false;
}

bool project_selector_setup::create_new_project_files() {
	const auto& user_input = gui.create_dialog;
	const auto& chosen_name = user_input.name;
	const auto& cloning_from = user_input.cloning_from;
	const auto sanitized = sanitization::sanitize_arena_path(EDITOR_PROJECTS_DIR, std::string(chosen_name));
	const auto sanitized_path = std::get_if<augs::path_type>(&sanitized);

	if (sanitized_path) {
		if (!cloning_from.empty()) {
			try {
				std::filesystem::copy(cloning_from, *sanitized_path, std::filesystem::copy_options::recursive);

				const auto old_filename = editor_project_paths(cloning_from).project_json.filename();
				const auto new_filename = editor_project_paths(*sanitized_path).project_json.filename();

				const auto old_project_json_in_new_project = (*sanitized_path) / old_filename;
				const auto new_project_json_in_new_project = (*sanitized_path) / new_filename;

				std::filesystem::rename(old_project_json_in_new_project, new_project_json_in_new_project);

				gui.projects_view.select_project(project_tab_type::MY_PROJECTS, *sanitized_path);
				scan_for_all_arenas();

				return true;
			}
			catch (...) {
				return false;
			}
		}
		else {
			augs::create_directory(*sanitized_path);
			scan_for_all_arenas();
			gui.projects_view.select_project(project_tab_type::MY_PROJECTS, *sanitized_path);

			const auto paths = editor_project_paths(*sanitized_path);

			const auto sanitized_name = sanitized_path->filename().string();
			const auto version = hypersomnia_version().get_version_string();
			const auto timestamp = augs::date_time::get_utc_timestamp();

			rapidjson::StringBuffer s;
			rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);

			{
				writer.StartObject();

				writer.Key("meta");

				{
					writer.StartObject();
					writer.Key("game_version");
					writer.String(version.c_str());
					writer.Key("name");
					writer.String(sanitized_name.c_str());
					writer.Key("version_timestamp");
					writer.String(timestamp.c_str());
					writer.EndObject();
				}

				writer.Key("about");

				{
					writer.StartObject();
					writer.Key("short_description");
					writer.String(user_input.short_description.c_str());
					writer.EndObject();
				}

				/* These aren't necessary because we're properly finding sensible defaults in the serializer */
	#if 0
				writer.Key("settings");

				{
					writer.StartObject();
					writer.Key("default_server_mode");
					writer.String("bomb_defusal");
					writer.EndObject();
				}

				writer.Key("quick_test");

				{
					writer.StartObject();
					writer.Key("mode");
					writer.String("quick_test");
					writer.EndObject();
				}
	#endif

				writer.EndObject();
			}
			
			augs::save_as_text(paths.project_json, s.GetString());

			return true;
		}
	}

	return false;
}

arena_identifier project_selector_setup::find_free_new_project_name() const {
	/* 
		In theory this could return an already existing string due to length limitation,
		but that would take millions of tests and would hang the program most probably.
	*/

	const auto path_template = (EDITOR_PROJECTS_DIR / "my_new_arena%x");
	return arena_identifier(augs::first_free_path(path_template).filename().string());
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

	{
		if (gui.create_dialog.perform(*this)) {
			gui.create_dialog.close();

			if (create_new_project_files()) {
				return custom_imgui_result::OPEN_SELECTED_PROJECT;
			}
		}
	}

	const auto result = gui.projects_view.perform(in);

	switch (result) {
		case project_list_view_result::NONE:
			return custom_imgui_result::NONE;

		case project_list_view_result::OPEN_CREATE_FROM_SELECTED_DIALOG:
			if (auto selected = gui.projects_view.find_selected()) {
				gui.create_dialog.name = std::string(selected->arena_name) + "_remake";
				gui.create_dialog.short_description = selected->about.short_description;
				gui.create_dialog.cloning_from = selected->arena_path;
				gui.create_dialog.open();
			}

			return custom_imgui_result::NONE;

		case project_list_view_result::OPEN_CREATE_DIALOG:
			gui.create_dialog.name = find_free_new_project_name();
			gui.create_dialog.short_description = "Resistance invades Metropolis.\nProtect the civilians.";
			gui.create_dialog.cloning_from.clear();
			gui.create_dialog.open();
			return custom_imgui_result::NONE;

		case project_list_view_result::OPEN_SELECTED_PROJECT:
			return custom_imgui_result::OPEN_SELECTED_PROJECT;
	}
}
