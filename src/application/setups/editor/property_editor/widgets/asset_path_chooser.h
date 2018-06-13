#pragma once
#include <vector>
#include <optional>
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "augs/misc/imgui/simple_browse_path_tree.h"
#include "augs/misc/imgui/path_tree_structs.h"
#include "view/maybe_official_path.h"
#include "application/setups/editor/property_editor/browsed_path_entry_base.h"
#include "application/setups/editor/property_editor/widgets/keyboard_acquiring_popup.h"

template <class I>
class asset_path_chooser : keyboard_acquiring_popup {
	using asset_widget_path_entry = browsed_path_entry_base<I>;
	using base = keyboard_acquiring_popup;

	std::vector<asset_widget_path_entry> all_paths;
	std::vector<asset_widget_path_entry> disallowed_paths;
	path_tree_settings tree_settings;

public:
	template <class F, class A>
	void perform(
		const std::string& label, 
		const maybe_official_path<I>& current_source,
		const augs::path_type& project_path,
		F on_choice,
		A allow_path_predicate,
		const std::string& disallowed_paths_displayed_name = ""
	) {
		using namespace augs::imgui;

		const auto displayed_str = current_source.get_prettified_full();

		if (auto combo = scoped_combo(label.c_str(), displayed_str.c_str(), ImGuiComboFlags_HeightLargest)) {
			if (base::check_opened_first_time()) {
				all_paths.clear();
				disallowed_paths.clear();

				auto make_path_adder = [&](const bool official, const auto& root) {
					return [official, this, &allow_path_predicate, &root](const auto& p) {
						if (maybe_official_path<I>::is_supported_extension(p.extension())) {
							auto cut_path = std::string(p.string());
							cut_preffix(cut_path, root.string() + "/");
							maybe_official_path<I> entry;

							entry.path = cut_path;
							entry.is_official = official;

							if (allow_path_predicate(entry)) {
								all_paths.emplace_back(std::move(entry));
							}
							else {
								disallowed_paths.emplace_back(std::move(entry));
							}
						}
					};
				};

				{
					const auto in_official_path = current_source.get_in_official();

					if (augs::exists(in_official_path)) {
						augs::for_each_in_directory_recursive(
							in_official_path,
							[](const auto&) {},
							make_path_adder(true, in_official_path)
						);
					}

					const auto in_project_path = project_path / current_source.get_content_suffix();

					if (augs::exists(in_project_path)) {
						augs::for_each_in_directory_recursive(
							in_project_path,
							[](const auto&) {},
							make_path_adder(false, in_project_path)
						);
					}
				}

				sort_range(all_paths);
				sort_range(disallowed_paths);
			}

			const bool acquire_keyboard = base::pop_acquire_keyboard();

			tree_settings.do_tweakers();

			simple_browse_path_tree(
				tree_settings,
				all_paths,
				[&](const auto& path_entry, const auto displayed_name) {
					const auto button_path = path_entry.get_full_path();
					const bool is_current = button_path == current_source;

					if (is_current && acquire_keyboard) {
						ImGui::SetScrollHere();
					}

					if (ImGui::Selectable(displayed_name.c_str(), is_current)) {
						ImGui::CloseCurrentPopup();

						on_choice(button_path);
					}
				},
				acquire_keyboard,
				disallowed_paths_displayed_name.size() > 0 ? disallowed_paths : decltype(disallowed_paths)(),
				disallowed_paths_displayed_name
			);
		}
		else {
			base::mark_not_opened();
		}
	}
};
