#pragma once
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/templates/identity_templates.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "game/assets/ids/is_asset_id.h"
#include "view/viewables/all_viewables_defs.h"

#include "augs/misc/imgui/path_tree_structs.h"
#include "application/setups/editor/gui/asset_browser_settings.h"
#include "application/setups/editor/property_editor/simple_browse_path_tree.h"

template <class F, class A>
void choose_asset_path(
	const std::string& label, 
	const augs::path_type& current_source_path,
	const augs::path_type& project_path,
	const std::string& suffix_folder,
	F on_choice,
	A allow_path_predicate,
	const std::string& disallowed_paths_displayed_name = ""
) {
	using namespace augs::imgui;

	using asset_control_path_entry = browsed_path_entry_base;

	thread_local bool acquire_once = true;
	thread_local int acquire_keyboard_times = 2;
	thread_local std::vector<asset_control_path_entry> all_paths;
	thread_local std::vector<asset_control_path_entry> disallowed_paths;
	thread_local path_tree_settings tree_settings;

	const auto displayed_str = 
		format_field_name(current_source_path.stem().string()) 
		+ " (" 
		+ augs::path_type(current_source_path).replace_filename("").string()
		+ ")"
	;

	if (auto combo = scoped_combo(label.c_str(), displayed_str.c_str(), ImGuiComboFlags_HeightLargest)) {
		if (acquire_once) {
			all_paths.clear();
			disallowed_paths.clear();

			auto path_adder = [&allow_path_predicate](const auto& p) {
				if (p.extension() == ".png") {
					if (allow_path_predicate(p)) {
						all_paths.emplace_back(p);
					}
					else {
						disallowed_paths.emplace_back(p);
					}
				}
			};

			{
				const auto official_gfx_path = typesafe_sprintf("content/official/%x", suffix_folder);
				const auto project_gfx_path = augs::path_type(project_path) += ("/" + suffix_folder);

				if (augs::exists(official_gfx_path)) {
					augs::for_each_in_directory_recursive(
						official_gfx_path,
						[](const auto&) {},
						path_adder
					);
				}

				if (augs::exists(project_gfx_path)) {
					augs::for_each_in_directory_recursive(
						project_gfx_path,
						[](const auto&) {},
						path_adder
					);
				}
			}

			sort_range(all_paths);
			sort_range(disallowed_paths);

			acquire_once = false;
			acquire_keyboard_times = 2;
		}

		const bool acquire_keyboard = acquire_keyboard_times > 0;

		if (acquire_keyboard_times > 0) {
			--acquire_keyboard_times;
		}

		tree_settings.do_tweakers();

		simple_browse_path_tree(
			tree_settings,
			all_paths,
			[&](const auto& path_entry, const auto displayed_name) {
				const auto button_path = path_entry.get_full_path();
				const bool is_current = button_path == current_source_path;

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
		acquire_once = true;
	}
}

struct asset_control_provider {
	all_viewables_defs& defs;
	const augs::path_type& project_path;
	editor_command_input in;

	template <class T>
	static constexpr bool handles = 
		//is_asset_id_v<T>
		is_one_of_v<T, assets::image_id>
	;

	auto describe_changed(
		const std::string& label,
		const assets::image_id from,
		const assets::image_id to
	) const {
		return description_pair {
			"",
			typesafe_sprintf("Set %x to %x", label, augs::to_display_path(defs.image_loadables[to].source_image_path))
		};
	}

	template <class T>
	bool handle(const std::string& label, T& object) const {
		bool changed = false;

		if constexpr(std::is_same_v<T, assets::image_id>) {
			const auto& current_source_path  = defs.image_loadables[object].source_image_path;

			auto on_choice = [&](const auto& chosen_path) {
				changed = true;

				auto& loadables = defs.image_loadables;

				if (const auto asset_id = ::find_asset_id_by_path(chosen_path, loadables)) {
					object = *asset_id;
				}
				else {
					auto& history = in.folder.history;

					{
						create_asset_id_command<assets::image_id> cmd;
						cmd.use_path = chosen_path.string();
						history.execute_new(std::move(cmd), in);
					}

					const auto* const last_addr = std::addressof(history.last_command());
					const auto* const cmd = std::get_if<create_asset_id_command<assets::image_id>>(last_addr);

					object = cmd->get_allocated_id();
				}
			};
			
			choose_asset_path(label, current_source_path, project_path, "gfx", on_choice, true_returner());
		}
		else {
			static_assert(!handles<T>, "Incomplete implementation!");
		}

		return changed;
	}
};
