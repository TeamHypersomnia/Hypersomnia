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
#include "augs/log.h"

const entropy_accumulator entropy_accumulator::zero;

constexpr auto miniature_size_v = 80;
constexpr auto preview_size_v = 250;

static auto push_selectable_colors(const rgba normal, const rgba hovered, const rgba active) {
	using namespace augs::imgui;

	return std::make_tuple(
		scoped_style_color(ImGuiCol_Header, normal),
		scoped_style_color(ImGuiCol_HeaderHovered, hovered),
		scoped_style_color(ImGuiCol_HeaderActive, active)
	);
}

static augs::path_type get_arenas_directory(const project_tab_type tab_type) {
	switch (tab_type) {
		case project_tab_type::MY_PROJECTS:
			return BUILDER_PROJECTS_DIR;

		case project_tab_type::OFFICIAL_ARENAS:
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

std::string project_list_entry::get_arena_name() const {
	return arena_path.filename().string();
}

std::optional<ad_hoc_atlas_subjects> project_selector_setup::get_new_ad_hoc_images() {
	if (rebuild_miniatures) {
		rebuild_miniatures = false;

		ad_hoc_atlas_subjects new_subjects;

		for (const auto& tab : gui.projects_view.tabs) {
			for (const auto& entry : tab.entries) {
				const auto blank_path = "content/necessary/gfx/blank_transparent.png";
				const auto miniature_path = entry.get_miniature_path();

				new_subjects.push_back({ entry.miniature_index, augs::exists(miniature_path) ? miniature_path : blank_path });
			}
		}

		return new_subjects;
	}

	return std::nullopt;
}

static auto default_meta(const augs::path_type& p) {
	builder_project_about out;

	if (p.filename() == "de_labs2") {
		out.credits.push_back({"Mapping & graphics", "lia"});
		out.credits.push_back({"Music", "Marcel Windys"});

		out.short_description = "Resistance has been tipped about sensitive information\nsitting in the depths of an abandoned rocket silo.";
		out.full_description = "This first map ever to be created by the community will test your CQB to the utmost limit.\nApart from a highly tactical gameplay, this map features a very creepy atmosphere -\nexpect uneasing ambience and lots of inexplicable sounds.";
	}

	if (p.filename() == "fy_minilab") {
		out.credits.push_back({"Mapping", "Patryk B. Czachurski"});
		out.credits.push_back({"Graphics", "Spicmir"});

		out.short_description = "A timeless 2017 classic and the first map ever created.";
		out.full_description = "Remember: true gentlemen don't pick up the rocket launcher.";
	}

	if (p.filename() == "de_cyberaqua") {
		out.credits.push_back({"Mapping", "Patryk B. Czachurski"});
		out.credits.push_back({"Graphics", "Spicmir"});

		out.short_description = "An experimental underwater biotech laboratory is under attack by a bombing squad.\nDon't let them lay hands on any expensive equipment.";
		out.full_description = "Protect the fish!";
	}

	return out;
}

void project_selector_setup::scan_for_all_arenas() {
	auto scan_for = [&](const project_tab_type type) {
		const auto source_directory = get_arenas_directory(type);

		auto register_arena = [&](const auto& arena_folder_path) {
			const auto paths = arena_paths(arena_folder_path);

			auto new_entry = project_list_entry();
			new_entry.arena_path = arena_folder_path;

			// TODO: give it a proper timestamp

			new_entry.timestamp = augs::date_time().secs_since_epoch();
			new_entry.miniature_index = miniature_index_counter++;
			new_entry.meta = default_meta(arena_folder_path);
			(void)paths;

			auto& view = gui.projects_view;
			view.tabs[type].entries.push_back(new_entry);

			return callback_result::CONTINUE; 
		};

		try {
			augs::for_each_in_directory(source_directory, register_arena, [](auto&&...) { return callback_result::CONTINUE; });
		}
		catch (const augs::filesystem_error& err) {
			//LOG("No folder: %x", source_directory);
		}
	};

	scan_for(project_tab_type::MY_PROJECTS);
	scan_for(project_tab_type::OFFICIAL_ARENAS);
	scan_for(project_tab_type::COMMUNITY_ARENAS);

	rebuild_miniatures = true;
}

project_selector_setup::project_selector_setup() {
	augs::create_directories(BUILDER_PROJECTS_DIR);

	load_gui_state();
	scan_for_all_arenas();
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

auto scoped_preserve_cursor() {
	const auto before_pos = ImGui::GetCursorPos();

	return augs::scope_guard([before_pos]() { ImGui::SetCursorPos(before_pos); });
};

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

	const auto avail = ImGui::GetContentRegionAvail();

	ImGui::Columns(num_columns);

	if (num_columns > 1) {
		ImGui::SetColumnWidth(0, avail.x * 0.7f);
	}

	do_column("Name");

	if (timestamp_column_name != std::nullopt) {
		do_column(*timestamp_column_name);
	}

	ImGui::Separator();

	const auto line_h = ImGui::GetTextLineHeight();

	for (const auto& entry : entries) {
		const auto& path = entry.arena_path;
		const auto arena_name = entry.get_arena_name();

		const auto selectable_size = ImVec2(0, 1 + miniature_size_v);
		auto id = scoped_id(entry.miniature_index);

		const bool is_selected = path == selected_arena_path;

		const auto local_pos = ImGui::GetCursorPos();

		{
			auto darkened_selectables = push_selectable_colors(
				rgba(255, 255, 255, 20),
				rgba(255, 255, 255, 30),
				rgba(255, 255, 255, 60)
			);

			if (ImGui::Selectable("##Entry", is_selected, ImGuiSelectableFlags_SpanAllColumns, selectable_size)) {
				selected_arena_path = path;
			}
		}

		const auto after_pos = ImGui::GetCursorPos();

		ImGui::SetCursorPos(local_pos);

		ImGui::SameLine();

		const auto image_padding = vec2(0, 0);
		const auto miniature_entry = mapped_or_nullptr(ad_hoc_in_atlas, entry.miniature_index);
		const auto target_miniature_size = vec2::square(miniature_size_v);

		if (miniature_entry != nullptr) {
			const auto miniature_size = miniature_entry->get_original_size();
			const auto resized_miniature_size = vec2i(vec2(miniature_size) * (static_cast<float>(miniature_size_v) / miniature_size.bigger_side()));

			const auto offset = (target_miniature_size - resized_miniature_size) / 2;
			game_image(*miniature_entry, resized_miniature_size, white, offset + image_padding, augs::imgui_atlas_type::AD_HOC);
		}

		invisible_button("", target_miniature_size + image_padding);

		ImGui::SameLine();

		const auto x = ImGui::GetCursorPosX();
		const auto prev_y = ImGui::GetCursorPosY() + line_h;

		text(arena_name);

		ImGui::SetCursorPosY(prev_y);
		ImGui::SetCursorPosX(x);

		text_disabled(entry.meta.short_description);

		ImGui::SetCursorPos(after_pos);

		if (num_columns > 1) {
			ImGui::NextColumn();

			const auto secs_ago = augs::date_time::secs_since_epoch() - entry.timestamp;
			text_disabled(augs::date_time::format_how_long_ago(true, secs_ago));

			ImGui::NextColumn();
		}
	}

	return false;
}

static auto selectable_with_icon(
	const augs::atlas_entry& icon,
	const std::string& label,
	const float size_mult,
	const float padding_mult,
	const rgba label_color,
	const std::array<rgba, 3> bg_cols
) {
	using namespace augs::imgui;

	const auto text_h = ImGui::GetTextLineHeight();
	const auto button_size = ImVec2(0, text_h * size_mult);

	shift_cursor(vec2(0, text_h * padding_mult));

	const auto before_pos = ImGui::GetCursorPos();

	bool result = false;

	{
		auto colored_selectable = push_selectable_colors(
			bg_cols[0],
			bg_cols[1],
			bg_cols[2]
		);

		auto id = scoped_id(label.c_str());

		result = ImGui::Selectable("###Button", true, ImGuiSelectableFlags_None, button_size);
	}

	{
		auto scope = scoped_preserve_cursor();

		ImGui::SetCursorPos(before_pos);

		const auto icon_size = icon.get_original_size();
		const auto icon_padding = vec2(icon_size);/// 1.5f;

		const auto image_offset = vec2(icon_padding.x, button_size.y / 2 - icon_size.y / 2);
		game_image(icon, icon_size, label_color, image_offset);

		const auto text_pos = vec2(before_pos) + image_offset + vec2(icon_size.x + icon_padding.x, icon_size.y / 2 - text_h / 2);
		ImGui::SetCursorPos(ImVec2(text_pos));
		text_color(label, label_color);
	}

	shift_cursor(vec2(0, text_h * padding_mult));

	return result;
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

custom_imgui_result projects_list_view::perform(const perform_custom_imgui_input in) {
	using namespace augs::imgui;

	auto left_buttons_column_size = ImGui::CalcTextSize("Community arenas  ");
	auto root = scoped_child("Selector main", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	(void)left_buttons_column_size;

	auto perform_arena_list = [&]() {
		auto& tab = tabs[current_tab];
		auto& ad_hoc = in.ad_hoc_in_atlas;

		switch (current_tab) {
			case project_tab_type::MY_PROJECTS:
			return tab.perform_list(ad_hoc, "Last modified");

			case project_tab_type::OFFICIAL_ARENAS:
			return tab.perform_list(ad_hoc, "Last updated");

			case project_tab_type::COMMUNITY_ARENAS:
			return tab.perform_list(ad_hoc, "When downloaded");

			default:
			return false;
		}
	};

	centered_text("Arena Builder 2.0 - Project Manager");

	const auto button_size_mult = 3.0f;
	const auto button_padding_mult = 0.5f;

	const bool create_pressed = selectable_with_icon(
		in.necessary_images[assets::necessary_image_id::EDITOR_ICON_CREATE],
		"CREATE NEW ARENA",
		button_size_mult,
		button_padding_mult,
		rgba(100, 255, 100, 255),
		{
			rgba(10, 50, 10, 255),
			rgba(20, 70, 20, 255),
			rgba(20, 90, 20, 255)
		}
	);

	if (create_pressed) {
		current_tab = project_tab_type::MY_PROJECTS;
	}

	{
		do_pretty_tabs(current_tab);

		const auto avail = ImGui::GetContentRegionAvail();
		const auto proj_list_width = avail.x * 0.6f;
		const auto proj_desc_width = avail.x * 0.4f;

		const auto text_h = ImGui::GetTextLineHeight();
		const auto space_for_clone_button = text_h * (button_padding_mult * 2 + button_size_mult);

		thread_local ImGuiTextFilter filter;
		filter.Draw();

		{
			auto scope = scoped_child("Project list view", ImVec2(proj_list_width, -space_for_clone_button));

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

		ImGui::SameLine();

		auto& tab = tabs[current_tab];
		const auto selected_entry = tab.find_selected();

		{
			auto fix_background_color = scoped_style_color(ImGuiCol_ChildBg, ImVec4{0.0f, 0.0f, 0.0f, 0.0f});

			auto scope = scoped_child("Project description view", ImVec2(proj_desc_width, -space_for_clone_button), false);

			if (selected_entry != nullptr) {
				auto& entry = *selected_entry;
				auto& meta = entry.meta;
				(void)meta;

				const auto image_padding = vec2(5, 5);
				const auto image_internal_padding = vec2i(15, 15);
				const auto preview_entry = mapped_or_nullptr(in.ad_hoc_in_atlas, entry.miniature_index);
				const auto target_preview_size = vec2::square(preview_size_v);

				if (preview_entry != nullptr) {
					const auto preview_size = preview_entry->get_original_size();
					const auto resized_preview_size = vec2i(vec2(preview_size) * (static_cast<float>(preview_size_v) / preview_size.bigger_side()));

					const auto offset = (target_preview_size - resized_preview_size) / 2;

					auto bg_size = target_preview_size + image_internal_padding * 2;
					bg_size.x = ImGui::GetContentRegionAvail().x;

					game_image(in.necessary_images[assets::necessary_image_id::BLANK], bg_size, rgba(0, 0, 0, 100), image_padding, augs::imgui_atlas_type::GAME);
					game_image(*preview_entry, resized_preview_size, white, offset + image_padding + image_internal_padding, augs::imgui_atlas_type::AD_HOC);

					invisible_button("", target_preview_size + image_padding + image_internal_padding * 2);

					auto fix_background_color = scoped_style_color(ImGuiCol_ChildBg, ImVec4{0.0f, 0.0f, 0.0f, 0.0f});

					shift_cursor(image_padding);

					auto scope = scoped_child("descview", ImVec2(0, 0), true);

					text(entry.get_arena_name() + "\n\n");

					ImGui::Columns(2);

					float max_w = 0;

					for (const auto& r : meta.credits) {
						max_w = std::max(max_w, ImGui::CalcTextSize(r.role.c_str()).x);
					}

					ImGui::SetColumnWidth(0, max_w + text_h);

					for (const auto& r : meta.credits) {
						text_disabled(std::string(r.role) + ": ");
						ImGui::NextColumn();

						text(r.person);
						ImGui::NextColumn();
					}

					ImGui::Columns(1);

					text("\n");

					ImGui::PushTextWrapPos();

					text_color(meta.full_description, rgba(210, 210, 210, 255));

					ImGui::PopTextWrapPos();
				}
			}
			else {
				text_disabled("(No project selected)");
			}
		}

		if (selected_entry != nullptr) {
			const bool is_template = current_tab != project_tab_type::MY_PROJECTS;

			const auto label = 
				is_template ?
				"CLONE TEMPLATE" :
				"OPEN"
			;

			const auto icon = 
				is_template ? 
				assets::necessary_image_id::EDITOR_ICON_CLONE : 
				assets::necessary_image_id::EDITOR_ICON_OPEN
			;

			const bool bottom_button_pressed = selectable_with_icon(
				in.necessary_images[icon],
				label,
				button_size_mult,
				button_padding_mult,
				rgba(120, 220, 255, 255),
				{
					rgba(15, 40, 70, 255),
					rgba(35, 60, 90, 255),
					rgba(55, 80, 110, 255)
				}
			);

			if (bottom_button_pressed) {
				if (is_template) {

				}
				else {
					return custom_imgui_result::OPEN_PROJECT;
				}
			}
		}
	}

	return custom_imgui_result::NONE;
}

augs::path_type projects_list_view::get_selected_project_path() const {
	return tabs[current_tab].selected_arena_path;
}

augs::path_type project_selector_setup::get_selected_project_path() const {
	return gui.projects_view.get_selected_project_path();
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

	return gui.projects_view.perform(in);
}

void project_selector_setup::load_gui_state() {
	// TODO: Read/write as lua

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
