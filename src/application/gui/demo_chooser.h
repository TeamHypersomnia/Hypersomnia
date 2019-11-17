#pragma once
#include <vector>
#include <optional>
#include <unordered_map>
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/imgui/simple_browse_path_tree.h"
#include "augs/misc/imgui/path_tree_structs.h"
#include "augs/misc/maybe_official_path.h"
#include "application/setups/editor/property_editor/browsed_path_entry_base.h"
#include "application/setups/editor/property_editor/widgets/keyboard_acquiring_popup.h"
#include "view/asset_funcs.h"
#include "augs/misc/time_utils.h"
#include "augs/log.h"

class demo_chooser : keyboard_acquiring_popup {
	using I = std::string;

	using base = keyboard_acquiring_popup;

	using asset_widget_path_entry = browsed_path_entry_base<I>;

	std::vector<asset_widget_path_entry> all_paths;
	path_tree_settings tree_settings;

	std::unordered_map<augs::path_type, std::string> write_times;

public:
	template <class F>
	void perform(
		const std::string& label, 
		const augs::path_type& current_source,
		const augs::path_type& demo_folder_path,
		F on_choice
	) {
		using namespace augs::imgui;

		const auto displayed_str = current_source.empty() ? std::string("(None selected)") : current_source.string();

		if (auto combo = scoped_combo(label.c_str(), displayed_str.c_str(), ImGuiComboFlags_HeightLargest)) {
			if (base::check_opened_first_time()) {
				write_times.clear();
				all_paths.clear();

				auto path_adder = [this](const auto& full_path) {
					if (full_path.extension() != ".dem") {
						return callback_result::CONTINUE;
					}

					maybe_official_path<I> entry;
					entry.path = full_path;

					try {
						write_times[full_path] = augs::date_time(augs::last_write_time(full_path)).how_long_ago();
					}
					catch (const augs::filesystem_error&) {
						write_times[full_path] = augs::date_time();
					}

					all_paths.emplace_back(std::move(entry));
					return callback_result::CONTINUE;
				};

				{
					if (augs::exists(demo_folder_path)) {
						augs::for_each_in_directory(
							demo_folder_path,
							[](const auto&) { return callback_result::CONTINUE; },
							path_adder
						);
					}
				}

				sort_range(all_paths);
				reverse_range(all_paths);
			}

			const bool acquire_keyboard = base::pop_acquire_keyboard();

			path_tree_settings settings;
			settings.linear_view = true;
			settings.pretty_names = false;

			simple_browse_path_tree(
				settings,
				all_paths,
				[&](const auto& path_entry, const auto displayed_name) {
					const auto button_path = path_entry.get_full_path();
					const bool is_current = button_path.path == current_source;

					if (is_current && acquire_keyboard) {
						ImGui::SetScrollHere();
					}

					if (ImGui::Selectable(displayed_name.c_str(), is_current)) {
						ImGui::CloseCurrentPopup();

						LOG("Choosing button path: %x ", button_path);
						on_choice(button_path);
					}
				},
				acquire_keyboard,
				{},
				"",
				{ "Demo", "When recorded" },
				[&](const auto& p) {
					return write_times.at(p.get_full_path().path);
				}
			);
		}
		else {
			base::mark_not_opened();
		}
	}
};
