#pragma once
#include <vector>
#include <optional>
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/imgui/simple_browse_path_tree.h"
#include "augs/misc/imgui/path_tree_structs.h"
#include "application/setups/debugger/property_debugger/browsed_path_entry_base.h"
#include "application/setups/debugger/property_debugger/widgets/keyboard_acquiring_popup.h"
#include "view/asset_funcs.h"
#include "augs/log.h"
#include "augs/string/path_sanitization.h"
#include "application/setups/debugger/detail/maybe_different_colors.h"

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

	std::vector<entry> other_paths;

	path_tree_settings tree_settings;

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

		if (auto combo = scoped_combo(label.c_str(), displayed_str.c_str(), ImGuiComboFlags_HeightLargest)) {
			if (base::check_opened_first_time()) {
				if (info) {
					other_paths.clear();

					for (auto& a : info->arenas_on_disk) {
						other_paths.push_back({ a.operator std::string() });
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
				}
			}

			const bool acquire_keyboard = base::pop_acquire_keyboard();

			thread_local ImGuiTextFilter filter;

			if (acquire_keyboard) {
				ImGui::SetKeyboardFocusHere();
			}

			filter.Draw();

			auto files_view = scoped_child("Files view", ImVec2(0, 20 * ImGui::GetTextLineHeightWithSpacing()));

			const std::array<std::string, 2> custom_column_names = { "Arena", "Type" };

			const auto avail = ImGui::GetContentRegionAvail();

			ImGui::Columns(2);

			text_disabled(custom_column_names[0]);
			ImGui::NextColumn();
			text_disabled(custom_column_names[1]);
			ImGui::NextColumn();

			ImGui::SetColumnWidth(0, avail.x - ImGui::CalcTextSize("User project9999").x);

			auto do_selectable = [&](const auto& path_entry) {
				const auto button_path = path_entry.path;
				const auto arena_name = button_path.filename().string();

				/* 
					TODO: this will behave wrongly when there is more than one area with the same name. 
					But we don't yet have proper logic to choose a specific area when there is a duplicate,
					i.e. we anyway for now default to official > user project > downloaded.
				*/

				const bool is_current = arena_name == current_source;

				if (is_current && acquire_keyboard) {
					ImGui::SetScrollHereY();
				}

				bool enabled = sanitization::arena_name_safe(arena_name);
				auto disabled = maybe_disabled_cols({}, !enabled);
				auto cond = cond_scoped_style_color(!enabled, ImGuiCol_Text, ImVec4(rgba(150, 40, 40, 255)));

				if (ImGui::Selectable(arena_name.c_str(), is_current, ImGuiSelectableFlags_SpanAllColumns)) {
					if (enabled) {
						ImGui::CloseCurrentPopup();

						LOG("Arena selected: %x", button_path);
						on_choice(button_path);
					}
				}
			};

			auto idx = 0;

			auto perform_paths = [&](const auto& label, const auto& entries, const auto type_name) {
				if (entries.empty()) {
					return;
				}

				auto scope = scoped_id(idx++);

				ImGui::Separator();
				text_disabled(label);

				ImGui::NextColumn();
				ImGui::NextColumn();

				for (const auto& entry : entries) {
					const auto arena_name = entry.get_filename().string();

					if (!filter.PassFilter(arena_name.c_str())) {
						continue;
					}

					do_selectable(entry);
					ImGui::NextColumn();
					text_disabled(type_name);
					ImGui::NextColumn();
				}
			};

			if (info) {
				perform_paths("(Arenas on server)", other_paths, "Arena on server");
			}
			else {
				perform_paths("(User projects)", user_projects_paths, "User project");
				perform_paths("(Downloaded arenas)", downloaded_paths, "Downloaded");
				perform_paths("(Official arenas)", official_paths, "Official");
			}
		}
		else {
			base::mark_not_opened();
		}
	}
};
