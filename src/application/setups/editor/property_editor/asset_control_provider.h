#pragma once
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/templates/identity_templates.h"
#include "augs/filesystem/file.h"
#include "augs/filesystem/directory.h"
#include "game/assets/ids/is_asset_id.h"
#include "view/viewables/all_viewables_defs.h"

#include "augs/misc/imgui/browse_path_tree.h"

struct asset_control_provider {
	all_viewables_defs& defs;
	const augs::path_type& project_path;

	struct sorted_path_entry {
		augs::path_type p;

		sorted_path_entry() = default;
		sorted_path_entry(
			const augs::path_type& p
		) : p(p)
		{}

		bool operator<(const sorted_path_entry& b) const {
			return p < b.p;
		}

		auto get_filename() const {
			return p.filename();
		}

		auto get_directory() const {
			return augs::path_type(p).replace_filename("").string();
		}

		const auto& get_full_path() const {
			return p;
		}
	};

	template <class T>
	static constexpr bool handles = 
		//is_asset_id_v<T>
		is_one_of_v<T, assets::image_id>
	;

	template <class T>
	bool handle(const std::string& label, const T& object) {
		using namespace augs::imgui;

		bool changed = false;

		if constexpr(std::is_same_v<T, assets::image_id>) {
			const auto& current_loadable = defs.image_loadables[object];
			const auto& current_source_path = current_loadable.source_image_path;

			const auto displayed_str = 
				format_field_name(current_source_path.stem().string()) 
				+ " (" 
				+ augs::path_type(current_source_path).replace_filename("").string()
				+ ")"
			;

			thread_local bool acquire_once = true;
			thread_local std::vector<sorted_path_entry> all_paths;
			thread_local path_tree_settings browser_settings;

			if (auto combo = scoped_combo(label.c_str(), displayed_str.c_str(), ImGuiComboFlags_HeightLargest)) {
				if (acquire_once) {
					all_paths.clear();

					auto path_adder = [](const auto& p) {
						if (p.extension() == ".png") {
							all_paths.emplace_back(p);
						}
					};

					{
						const auto official_gfx_path = "content/official/gfx";
						const auto project_gfx_path = augs::path_type(project_path) += "/gfx";

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

					acquire_once = false;
				}

				browse_path_tree(
					browser_settings,
					all_paths,
					[&](const auto& path_entry, const auto displayed_name) {
						const bool is_current = path_entry.get_full_path() == current_source_path;

						if (ImGui::Selectable(displayed_name.c_str(), is_current)) {
							changed = true;
						}
					}
				);
			}
			else {
				acquire_once = true;
			}
		}
		else {
			static_assert(!handles<T>, "Incomplete implementation!");
		}

		return changed;
	}
};
