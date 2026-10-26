#pragma once
#include <vector>
#include <algorithm>
#include <optional>
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/imgui/simple_browse_path_tree.h"
#include "augs/misc/imgui/path_tree_structs.h"
#include "application/gui/browsed_path_entry_base.h"
#include "application/gui/keyboard_acquiring_popup.h"
#include "view/asset_funcs.h"
#include "augs/log.h"
#include "augs/string/path_sanitization.h"
#include "augs/misc/imgui/maybe_different.h"
#include "augs/misc/imgui/imgui_miniature_combo.h"
#include "application/gui/arena_miniatures.h"
#include "application/gui/pretty_tabs.h"

enum class arena_chooser_tab {
	// GEN INTROSPECTOR enum class arena_chooser_tab
	OFFICIAL,
	DOWNLOADED,
	USER_PROJECTS,

	COUNT
	// END GEN INTROSPECTOR
};

class arena_chooser : keyboard_acquiring_popup {
	using I = std::string;

	using base = keyboard_acquiring_popup;

	using asset_widget_path_entry = browsed_path_entry_base<I>;

	struct entry {
		augs::path_type path;

		bool operator<(const entry& b) const {
			return augs::natural_order(path.string(), b.path.string());
		}

		auto get_filename() const {
			return path.filename();
		}

		auto get_displayed_directory() const {
			return augs::path_type(path).replace_filename("").string();
		}

		const auto& get_full_path() const {
			return path;
		}
	};

	std::vector<entry> official_paths;
	std::vector<entry> downloaded_paths;
	std::vector<entry> user_projects_paths;

	/*
		A dedicated server's custom maps (custom/maps): apart, under a header of their own.
	*/

	std::vector<entry> custom_paths;

	std::vector<entry> other_paths;

	path_tree_settings tree_settings;

	arena_chooser_tab active_tab = arena_chooser_tab::OFFICIAL;

public:
	template <class F>
	void perform(
		const std::string& label, 
		const std::string& current_source,
		const augs::path_type& official_folder,
		const augs::path_type& downloaded_folder,
		const augs::path_type& user_projects_folder,
		const server_runtime_info* info,
		F on_choice
	) {
		using namespace augs::imgui;

		const auto displayed_str = current_source.empty() ? std::string("(Invalid)") : current_source;
		const auto* const miniature = ::get_arena_miniatures().request(current_source);

		if (auto combo = scoped_miniature_combo(label, displayed_str, miniature, augs::imgui_atlas_type::ARENA_MINIATURES)) {
			if (base::check_opened_first_time()) {
				::get_arena_miniatures().request_again_changed_on_disk();

				if (info) {
					other_paths.clear();
					custom_paths.clear();

					for (auto& a : info->arenas_on_disk) {
						const bool custom = std::find(info->custom_arenas_on_disk.begin(), info->custom_arenas_on_disk.end(), a) != info->custom_arenas_on_disk.end();
						(custom ? custom_paths : other_paths).push_back({ a.operator std::string() });
					}
				}
				else {
					auto add_paths = [&](const auto& root, auto& out_entries) {
						out_entries.clear();

						try {
							augs::for_each_in_directory(
								root,
								[&](const auto& p) {
									if (::ends_with(p.string(), ".part") || ::ends_with(p.string(), ".old")) {
										return callback_result::CONTINUE;
									}

									out_entries.push_back({ std::filesystem::relative(p, root) });
									return callback_result::CONTINUE;
								},
								[](const auto&) { return callback_result::CONTINUE; }
							);
						}
						catch (...) {

						}

						sort_range(out_entries);
					};

					add_paths(official_folder, official_paths);
					add_paths(downloaded_folder, downloaded_paths);
					add_paths(user_projects_folder, user_projects_paths);

					/*
						Open on the tab of the current arena - official first, as an arena is looked for.
					*/

					auto has_current = [&](const auto& entries) {
						return std::any_of(entries.begin(), entries.end(), [&](const auto& e) { return e.get_filename().string() == current_source; });
					};

					if (has_current(official_paths)) {
						active_tab = arena_chooser_tab::OFFICIAL;
					}
					else if (has_current(user_projects_paths)) {
						active_tab = arena_chooser_tab::USER_PROJECTS;
					}
					else if (has_current(downloaded_paths)) {
						active_tab = arena_chooser_tab::DOWNLOADED;
					}
				}
			}

			const bool acquire_keyboard = base::pop_acquire_keyboard();

			thread_local ImGuiTextFilter filter;

			if (acquire_keyboard) {
				ImGui::SetKeyboardFocusHere();
			}

			filter.Draw();

			/*
				Typing in the filter searches all the tabs at once - picking a tab then clears it.
			*/

			const bool filtering = filter.IsActive();

			if (info == nullptr) {
				const auto tab_before = active_tab;

				do_pretty_tabs(active_tab);

				if (filtering && active_tab != tab_before) {
					filter.Clear();
				}
			}

			const bool show_type = info == nullptr && filtering;

			auto files_view = scoped_child("Files view", ImVec2(0, 24 * ImGui::GetTextLineHeightWithSpacing()));

			const auto line_height = ImGui::GetTextLineHeight();
			const auto row_height = ImGui::GetFrameHeight() * 2.f;

			/*
				A column of miniatures on a darker background, the name and the type of each arena right of it.
			*/

			const auto table_flags = ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_NoPadOuterX;

			auto border_like_separator = scoped_style_color(ImGuiCol_TableBorderLight, ImGui::GetStyleColorVec4(ImGuiCol_Separator));

			/*
				No vertical cell padding - rows exactly row_height tall, the empty last row of no height.
			*/

			auto no_vertical_cell_padding = scoped_style_var(ImGuiStyleVar_CellPadding, ImVec2(ImGui::GetStyle().CellPadding.x, 0.f));

			if (!ImGui::BeginTable(show_type ? "Arenas with types" : "Arenas", show_type ? 3 : 2, table_flags)) {
				return;
			}

			auto end_table = augs::scope_guard([]() { ImGui::EndTable(); });

			ImGui::TableSetupColumn("Miniature", ImGuiTableColumnFlags_WidthFixed, row_height);
			ImGui::TableSetupColumn("Arena", ImGuiTableColumnFlags_WidthStretch);

			if (show_type) {
				ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, ImGui::CalcTextSize("User project9").x);
			}

			auto darken_miniature_column = [&]() {
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, miniature_background_color(), 0);
			};

			auto indent_name = [&]() {
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetStyle().ItemSpacing.x * 2);
			};

			auto centered_vertically = [&]() {
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (row_height - line_height) / 2);
			};

			ImGui::TableNextRow();
			darken_miniature_column();
			ImGui::TableSetColumnIndex(1);
			indent_name();
			text_disabled("Arena");

			if (show_type) {
				ImGui::TableSetColumnIndex(2);
				text_disabled("Type");
			}

			auto do_row = [&](const auto& path_entry, const std::string& arena_name, const auto type_name) {
				const auto& button_path = path_entry.path;

				/* 
					TODO: this will behave wrongly when there is more than one area with the same name. 
					But we don't yet have proper logic to choose a specific area when there is a duplicate,
					i.e. we anyway for now default to official > user project > downloaded.
				*/

				const bool is_current = arena_name == current_source;

				ImGui::TableNextRow(ImGuiTableRowFlags_None, row_height);
				darken_miniature_column();
				ImGui::TableSetColumnIndex(0);

				if (is_current && acquire_keyboard) {
					ImGui::SetScrollHereY();
				}

				bool enabled = sanitization::arena_name_safe(arena_name);
				auto disabled = maybe_disabled_cols({}, !enabled);
				auto cond = cond_scoped_style_color(!enabled, ImGuiCol_Text, ImVec4(rgba(150, 40, 40, 255)));

				const auto cell_lt = vec2(ImGui::GetCursorScreenPos());
				const auto cell_size = vec2(ImGui::GetContentRegionAvail().x, row_height);

				auto row_id = scoped_id(arena_name.c_str());

				const bool chosen = [&]() {
					auto subtle_highlight = scoped_selectable_colors({
						rgba(255, 255, 255, 20),
						rgba(255, 255, 255, 30),
						rgba(255, 255, 255, 60)
					});

					return ImGui::Selectable("##row", is_current, ImGuiSelectableFlags_SpanAllColumns, ImVec2(0, row_height));
				}();

				miniature_image(
					*ImGui::GetWindowDrawList(),
					::get_arena_miniatures().request(arena_name),
					cell_lt,
					cell_size,
					2.f,
					augs::imgui_atlas_type::ARENA_MINIATURES
				);

				ImGui::TableSetColumnIndex(1);
				centered_vertically();
				indent_name();
				text(arena_name);

				if (show_type) {
					ImGui::TableSetColumnIndex(2);
					centered_vertically();
					text_disabled(type_name);
				}

				if (chosen) {
					if (enabled) {
						ImGui::CloseCurrentPopup();

						LOG("Arena selected: %x", button_path);
						on_choice(button_path);
					}
				}
			};

			auto idx = 0;
			auto num_shown = 0;

			auto perform_paths = [&](const auto& entries, const auto type_name) {
				auto scope = scoped_id(idx++);

				for (const auto& entry : entries) {
					const auto arena_name = entry.get_filename().string();

					if (!filter.PassFilter(arena_name.c_str())) {
						continue;
					}

					do_row(entry, arena_name, type_name);
					++num_shown;
				}
			};

			if (info) {
				perform_paths(other_paths, "Arena on server");

				if (!custom_paths.empty()) {
					ImGui::TableNextRow(ImGuiTableRowFlags_None, row_height);
					darken_miniature_column();
					ImGui::TableSetColumnIndex(1);
					centered_vertically();
					indent_name();
					text_disabled("Custom maps");

					perform_paths(custom_paths, "Custom map");
				}
			}
			else if (filtering) {
				perform_paths(official_paths, "Official");
				perform_paths(downloaded_paths, "Downloaded");
				perform_paths(user_projects_paths, "User project");
			}
			else {
				switch (active_tab) {
					case arena_chooser_tab::OFFICIAL:
						perform_paths(official_paths, "Official");
						break;
					case arena_chooser_tab::DOWNLOADED:
						perform_paths(downloaded_paths, "Downloaded");
						break;
					case arena_chooser_tab::USER_PROJECTS:
						perform_paths(user_projects_paths, "User project");
						break;
					default:
						break;
				}
			}

			if (num_shown == 0) {
				ImGui::TableNextRow(ImGuiTableRowFlags_None, row_height);
				darken_miniature_column();
				ImGui::TableSetColumnIndex(1);
				centered_vertically();
				indent_name();
				text_disabled(filtering ? "(No matching arenas)" : "(No arenas)");
			}

			/*
				An empty row last, of no height - so that the last arena, too, has a separator under it
				(an inner border is only between rows), with the column border ending right on it.
			*/

			ImGui::TableNextRow(ImGuiTableRowFlags_None, 0.f);
		}
		else {
			base::mark_not_opened();
		}
	}
};
