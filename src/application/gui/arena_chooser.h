#pragma once
#include <vector>
#include <optional>
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/imgui/simple_browse_path_tree.h"
#include "augs/misc/imgui/path_tree_structs.h"
#include "augs/misc/maybe_official_path.h"
#include "application/setups/editor/property_editor/browsed_path_entry_base.h"
#include "application/setups/editor/property_editor/widgets/keyboard_acquiring_popup.h"
#include "view/asset_funcs.h"
#include "augs/log.h"

class arena_chooser : keyboard_acquiring_popup {
	using I = std::string;

	using base = keyboard_acquiring_popup;

	using asset_widget_path_entry = browsed_path_entry_base<I>;

	std::vector<asset_widget_path_entry> all_paths;
	path_tree_settings tree_settings;

public:
	template <class F>
	void perform(
		const std::string& label, 
		const std::string& current_source,
		const augs::path_type& official_path,
		const augs::path_type& community_path,
		F on_choice
	) {
		using namespace augs::imgui;

		const auto displayed_str = current_source.empty() ? std::string("(Invalid)") : current_source;

		if (auto combo = scoped_combo(label.c_str(), displayed_str.c_str(), ImGuiComboFlags_HeightLargest)) {
			if (base::check_opened_first_time()) {
				all_paths.clear();

				auto make_path_adder = [&](const bool official, const auto& root) {
					return [official, this, &root](const auto& p) {
						maybe_official_path<I> entry;

						entry.path = std::filesystem::relative(p, root);
						entry.is_official = official;

						all_paths.emplace_back(std::move(entry));
						return callback_result::CONTINUE;
					};
				};

				{
					if (augs::exists(official_path)) {
						augs::for_each_in_directory(
							official_path,
							make_path_adder(true, official_path),
							[](const auto&) { return callback_result::CONTINUE; }
						);
					}

					if (augs::exists(community_path)) {
						augs::for_each_in_directory(
							community_path,
							make_path_adder(false, community_path),
							[](const auto&) { return callback_result::CONTINUE; }
						);
					}
				}

				sort_range(all_paths);
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
					const bool is_current = button_path.path.string() == current_source;

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
				{ "Arena", "Type" }
			);
		}
		else {
			base::mark_not_opened();
		}
	}
};
