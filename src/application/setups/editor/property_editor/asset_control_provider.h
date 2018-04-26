#pragma once
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/templates/identity_templates.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "view/viewables/all_viewables_defs.h"

#include "augs/misc/imgui/path_tree_structs.h"
#include "application/setups/editor/property_editor/browsed_path_entry_base.h"
#include "application/setups/editor/gui/asset_browser_settings.h"
#include "application/setups/editor/property_editor/simple_browse_path_tree.h"
#include "application/setups/editor/property_editor/tweaker_type.h"

template <class I, class F, class A>
void choose_asset_path(
	const std::string& label, 
	const maybe_official_path<I>& current_source,
	const augs::path_type& project_path,
	F on_choice,
	A allow_path_predicate,
	const std::string& disallowed_paths_displayed_name = ""
) {
	using namespace augs::imgui;

	using asset_control_path_entry = browsed_path_entry_base<I>;

	thread_local bool acquire_once = true;
	thread_local int acquire_keyboard_times = 2;
	thread_local std::vector<asset_control_path_entry> all_paths;
	thread_local std::vector<asset_control_path_entry> disallowed_paths;
	thread_local path_tree_settings tree_settings;

	const auto displayed_str = current_source.to_display_prettified();

	if (auto combo = scoped_combo(label.c_str(), displayed_str.c_str(), ImGuiComboFlags_HeightLargest)) {
		if (acquire_once) {
			all_paths.clear();
			disallowed_paths.clear();

			auto make_path_adder = [&](const bool official, const auto& root) {
				return [&](const auto& p) {
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
		acquire_once = true;
	}
}

struct asset_sane_default_provider {
	all_viewables_defs& defs;

	template <class T>
	auto construct() const {
		if constexpr(std::is_same_v<T, invariants::sprite>) {
			auto& definitions = defs.image_definitions;
			invariants::sprite t;
			t.tex = definitions.get_nth_id(0);
			return t;
		}

		return T();
	}
};

struct asset_control_provider {
	all_viewables_defs& defs;
	const augs::path_type& project_path;
	editor_command_input in;

	template <class T>
	static constexpr bool handles = 
		is_one_of_v<T, assets::image_id, assets::sound_id>
	;

	template <class T>
	auto describe_changed(
		const std::string& formatted_label,
		const T to
	) const {
		return typesafe_sprintf("Set %x to %x", formatted_label, augs::to_display(get_viewable_pool<T>(defs)[to].get_source_path().path));
	}

	template <class T>
	std::optional<tweaker_type> handle(const std::string& identity_label, T& object) const {
		bool changed = false;

		if constexpr(handles<T>) {
			auto& definitions = get_viewable_pool<T>(defs);

			auto on_choice = [&](const auto& chosen_path) {
				changed = true;

				if (const auto asset_id = ::find_asset_id_by_path(chosen_path, definitions)) {
					object = *asset_id;
				}
				else {
					auto& history = in.folder.history;

					{
						create_asset_id_command<T> cmd;
						cmd.use_path = chosen_path;
						history.execute_new(std::move(cmd), in);
					}

					const auto* const last_addr = std::addressof(history.last_command());
					const auto* const cmd = std::get_if<create_asset_id_command<T>>(last_addr);

					object = cmd->get_allocated_id();
				}
			};
			
			const auto& current_source  = definitions[object].get_source_path();
			choose_asset_path(identity_label, current_source, project_path, on_choice, true_returner());
		}
		else {
			static_assert(!handles<T>, "Incomplete implementation!");
		}

		if (changed) {
			return tweaker_type::DISCRETE;
		}

		return std::nullopt;
	}
};
