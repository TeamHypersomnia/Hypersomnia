#define INCLUDE_TYPES_IN 1

#include "application/setups/debugger/gui/debugger_pathed_asset_gui.h"

#if BUILD_PROPERTY_DEBUGGER
#include "augs/string/string_templates.h"

#include "augs/window_framework/window.h"
#include "augs/templates/list_utils.h"
#include "augs/templates/introspection_utils/field_name_tracker.h"
#include "augs/templates/introspection_utils/on_dynamic_content.h"

#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_image_color_picker.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_game_image.h"
#include "augs/misc/imgui/path_tree_structs.h"
#include "augs/misc/imgui/imgui_drawers.h"

#include "game/organization/for_each_entity_type.h"
#include "view/viewables/images_in_atlas_map.h"

#include "application/setups/debugger/debugger_folder.h"
#include "application/setups/debugger/debugger_history.hpp"
#include "application/intercosm.h"

#include "application/setups/debugger/detail/other_styles.h"
#include "application/setups/debugger/detail/format_struct_name.h"

#include "application/setups/debugger/property_debugger/widgets/asset_path_chooser.h"
#include "application/setups/debugger/property_debugger/widgets/image_color_picker_widget.h"
#include "application/setups/debugger/property_debugger/widgets/image_offset_widget.h"
#include "application/setups/debugger/property_debugger/widgets/source_path_widget.h"
#include "application/setups/debugger/property_debugger/widgets/non_standard_shape_widget.h"

#include "application/setups/debugger/detail/do_pathed_asset_properties.h"
#include "application/setups/debugger/detail/do_forget_button.h"

#if NDEBUG
#define BUILD_LOCATION_FINDERS 1
#else
#define BUILD_LOCATION_FINDERS 0
#endif

#if BUILD_LOCATION_FINDERS
#include "application/setups/debugger/detail/find_locations_that_use.h"
#endif
#include "application/setups/debugger/detail/checkbox_selection.h"
#include "application/setups/debugger/property_debugger/compare_all_fields_to.h"
#include "augs/readwrite/byte_readwrite.h"

#endif

template <class asset_id_type>
void debugger_pathed_asset_gui<asset_id_type>::set_currently_viewed(const asset_id_type id) {
	property_debugger_data.reset();
	separate_properties.show = true;
	separate_properties.currently_viewed = id;

	ImGui::SetWindowFocus("Current image");
}

template <class asset_id_type>
void debugger_pathed_asset_gui<asset_id_type>::perform(
	augs::window& window,
	const property_debugger_settings& settings,
	const images_in_atlas_map& game_atlas,
   	debugger_command_input cmd_in
) {
#if BUILD_PROPERTY_DEBUGGER
	constexpr bool is_image_type = std::is_same_v<asset_id_type, assets::image_id>;
	const bool show_properties_column = path_browser_settings.show_properties_column;

	using namespace augs::imgui;

	auto window_scope = base::make_scoped_window();

	if (!window_scope) {
		return;
	}

	auto& folder = cmd_in.folder;
	auto& work = folder.commanded->work;

	auto& viewables = work.viewables;
	auto& definitions = get_viewable_pool<asset_id_type>(viewables);

	using def_type = typename std::remove_reference_t<decltype(definitions)>::mapped_type;

	using asset_entry_type = pathed_asset_entry<asset_id_type>;

	thread_local std::unordered_set<augs::path_type> _last_seen_missing_paths;
	/* Linker error fix */
	auto& last_seen_missing_paths = _last_seen_missing_paths;
	thread_local std::vector<asset_entry_type> missing_orphaned_paths;
	thread_local std::vector<asset_entry_type> missing_paths;

	thread_local std::vector<asset_entry_type> orphaned_paths;
	thread_local std::vector<asset_entry_type> used_paths;

	auto for_each_range = [](auto callback) {
		callback(missing_orphaned_paths);
		callback(missing_paths);

		callback(orphaned_paths);
		callback(used_paths);
	};

	for_each_range([](auto& r) { r.clear(); });

	auto is_ticked = [this](const auto& p) {
		return found_in(ticked_assets, p.id);
	};

	auto get_all_ticked_and_existing = [&](auto in_range) {
		erase_if(in_range, [&] (const auto& candidate) { return !is_ticked(candidate); } );
		return in_range;
	};

	if (base::will_acquire_keyboard_once()) {
		acquire_missing_paths = true;
	}

	if (acquire_missing_paths) {
		last_seen_missing_paths.clear();
	}

	const auto label = std::string(assets::get_label<asset_id_type>());

	for_each_id_and_object(definitions,
		[&](const asset_id_type id, const def_type& object) mutable {
			const auto path = object.get_source_path();
			auto new_entry = asset_entry_type(path, id);

#if BUILD_LOCATION_FINDERS
			const auto in_locations = candidate_id_locations {
				work.world, 
				work.viewables,
				folder.commanded->rulesets
			};

			find_locations_that_use(id, in_locations, [&](const std::string& location) {
				new_entry.using_locations.push_back(location);
			});
#else
			new_entry.using_locations.emplace_back();
#endif

			auto push_missing = [&]() {
				auto& target_range = new_entry.used() ? missing_paths : missing_orphaned_paths;
				target_range.emplace_back(std::move(new_entry));
			};

			auto push_existing = [&]() {
				auto& target_range = new_entry.used() ? used_paths : orphaned_paths;
				target_range.emplace_back(std::move(new_entry));
			};

			auto lazy_check_missing = [&](const auto& p) {
				if (acquire_missing_paths) {
					if (!augs::exists(p)) {
						last_seen_missing_paths.emplace(p);
						return true;
					}

					return false;
				}

				return found_in(last_seen_missing_paths, p);
			};

			const auto& view = asset_definition_view<const def_type>(folder.current_path, object);

			if (lazy_check_missing(view.get_resolved_source_path())) {
				push_missing();
			}
			else {
				push_existing();
			}
		}
	);

	acquire_missing_paths = false;
	
	path_browser_settings.do_tweakers();
	ImGui::SameLine();
	checkbox("Properties window", separate_properties.show);
	ImGui::Separator();

	if (ImGui::Button("Re-check existence")) {
		acquire_missing_paths = true;
	}

	ImGui::Separator();

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	base::acquire_keyboard_once();

	auto& tree_settings = path_browser_settings.tree_settings;

	auto get_displayed_asset_name = [&](const auto& entry) {
		return tree_settings.pretty_names ? ::get_displayed_name(definitions[entry.id]) : entry.get_filename().string();
	};

	auto prepare = [&](auto& range){
		erase_if(range, [&](const auto& entry){
			const auto displayed_name = get_displayed_asset_name(entry);
			const auto displayed_dir = entry.get_displayed_directory();

			if (!filter.PassFilter(displayed_name.c_str()) && !filter.PassFilter(displayed_dir.c_str())) {
				return true;
			}

			return false;
		});

		sort_range(range);
	};

	for_each_range(prepare);

	if (!path_browser_settings.show_orphaned) {
		orphaned_paths.clear();
	}

	asset_entry_type* currently_viewed_entry = nullptr;
	std::vector<asset_entry_type>* range_of_currently_viewed = nullptr;

	auto look_for_current_in = [&](auto& paths) {
		for (auto& e : paths) {
			if (e.id == separate_properties.currently_viewed) {
				currently_viewed_entry = std::addressof(e);
				range_of_currently_viewed = std::addressof(paths);
			}
		}
	};

	look_for_current_in(missing_paths);
	look_for_current_in(missing_orphaned_paths);

	look_for_current_in(orphaned_paths);
	look_for_current_in(used_paths);

	auto handle_moving_of_currently_viewed = [&]() {
		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows | ImGuiFocusedFlags_RootWindow)) {
			if (move_currently_viewed_by != 0 && (orphaned_paths.size() + used_paths.size() > 0)) {
				std::optional<int> found_idx;

				for (auto& p : orphaned_paths) { 
					if (std::addressof(p) == currently_viewed_entry) {
						found_idx = index_in(orphaned_paths, p);
					}
				}

				for (auto& p : used_paths) { 
					if (std::addressof(p) == currently_viewed_entry) {
						found_idx = orphaned_paths.size() + index_in(used_paths, p);
					}
				}

				if (!found_idx) {
					found_idx = -1;
				}

				auto new_idx = *found_idx + move_currently_viewed_by;

				const auto all_n = orphaned_paths.size() + used_paths.size();

				if (new_idx < 0) {
					new_idx = all_n + new_idx;
				}

				new_idx %= all_n;

				if (new_idx < static_cast<std::ptrdiff_t>(orphaned_paths.size())) {
					currently_viewed_entry = orphaned_paths.data() + new_idx;
					range_of_currently_viewed = std::addressof(orphaned_paths);
				}
				else {
					currently_viewed_entry = used_paths.data() + new_idx - orphaned_paths.size();
					range_of_currently_viewed = std::addressof(used_paths);
				}

				move_currently_viewed_by = 0;
				set_currently_viewed(currently_viewed_entry->id);
			}
		}
	};

	handle_moving_of_currently_viewed();

	const auto prop_in = property_debugger_input { settings, property_debugger_data };

	separate_properties_focused = false;

	if (separate_properties.show && currently_viewed_entry != nullptr) {
		const auto& path_entry = *currently_viewed_entry;

		ImGui::SetNextWindowSize(ImVec2(350,560), ImGuiCond_FirstUseEver);

		const auto title = "Current " + label;
		auto scope = scoped_window(title.c_str(), std::addressof(separate_properties.show));

		separate_properties_focused = ImGui::IsWindowFocused() || ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

		handle_moving_of_currently_viewed();

		ImGui::Columns(2);

		const auto& asset_entries = *range_of_currently_viewed;
		const auto ticked_and_existing = get_all_ticked_and_existing(asset_entries);

		thread_local std::vector<asset_id_type> ticked_ids;
		ticked_ids.clear();

		for (const auto& p : ticked_and_existing) {
			ticked_ids.push_back(p.id);
		}

		const auto is_current_ticked = is_ticked(path_entry);

		do_pathed_asset_properties(
			tree_settings,
			prop_in,
			cmd_in,
			path_entry, 
			ticked_and_existing, 
			ticked_ids,
			is_current_ticked,
			game_atlas,
			preview,
			2,
			true
		);

		ImGui::Columns(1);
		ImGui::Separator();

		const auto resolved_path = path_entry.get_full_path().resolve(cmd_in.folder.current_path);

		if (ImGui::Button("Reveal in explorer")) {
			window.reveal_in_explorer(resolved_path);
		}

		text_disabled(typesafe_sprintf("%x", resolved_path.string()));

		if constexpr(is_image_type) {
			const auto& entry = game_atlas.find_or(currently_viewed_entry->id).diffuse;

			const bool is_missing =
				range_of_currently_viewed == std::addressof(missing_paths)
				|| range_of_currently_viewed == std::addressof(missing_orphaned_paths)
			;

			if (!is_missing) {
				if (!entry.exists()) {
					text_disabled("The preview is being loaded...");
				}
				else {
					const auto is = vec2i(entry.get_original_size());

					text_disabled(typesafe_sprintf("%x x %x pixels", is.x, is.y));

					game_image(entry, is);
					invisible_button("###imgpreview", is);
				}
			}
			else {
				text_color("File is missing! No preview available.", red);
				text_color("\nPut a proper file into the original location,\nor point the editor to a new file path and click \"Re-check existence\".", orange);
			}
		}

	}

	move_currently_viewed_by = 0;

	auto files_view = scoped_child("Files view");

	const bool show_using_locations = BUILD_LOCATION_FINDERS && path_browser_settings.show_using_locations;
	const auto num_cols = 2 + (show_using_locations ? 1 : 0) + (show_properties_column ? 1 : 0);

	if (tree_settings.linear_view) {
		ImGui::Columns(num_cols);

		bool official_separator = false;

		auto do_asset_entry = [&](const auto& path_entry, const auto& ticked_in_range, const auto& ticked_ids) {
			separator_if_unofficials_ended(path_entry.get_full_path(), official_separator);

			const auto id = path_entry.id;
			auto scope = scoped_id(id);

			const auto displayed_name = get_displayed_asset_name(path_entry);
			const auto displayed_dir = path_entry.get_displayed_directory();

			const auto is_current_ticked = is_ticked(path_entry);

			const auto tick_flags = do_selection_checkbox(ticked_assets, id, is_current_ticked, id);

			if (do_forget_button(
				cmd_in,
				path_entry,
				ticked_in_range,
				is_current_ticked,
				label
			)) {
				/* It has just been forgotten. */
				return;
			}

			auto total_flags = tick_flags;
		   
			if (!show_properties_column) {
				if (currently_viewed_entry == std::addressof(path_entry)) {
					total_flags |= ImGuiTreeNodeFlags_Bullet;
				}
				else {
					total_flags |= ImGuiTreeNodeFlags_Leaf;
				}
			}
		
			const auto node_label = displayed_name + "###" + (show_properties_column ? "Node" : "Leaf");
			const auto node = scoped_tree_node_ex(node_label, total_flags);

			if (!show_properties_column) {
				if (ImGui::IsItemClicked()) {
					set_currently_viewed(id);
				}
			}

			if constexpr(is_image_type) {
				const auto& entry = game_atlas.find_or(id).diffuse;

				if (entry.exists()) {
					if (ImGui::IsItemHovered()) {
						if (entry.was_successfully_packed) {
							auto tooltip = scoped_tooltip();
							game_image_button("###TooltipPreview", entry);
						}
					}

					if (entry.was_successfully_packed) {
						ImGui::SameLine();
						const auto size = entry.get_original_size();
						const auto size_suffix = typesafe_sprintf("(%xx%x)", size.x, size.y);
						text_disabled(size_suffix);
					}
				}
			}

			next_columns(show_properties_column ? 2 : 1);

			text_disabled(displayed_dir);

			if (show_using_locations) {
				ImGui::NextColumn();

				if (path_entry.used()) {
					const auto& using_locations = path_entry.using_locations;

					if (auto locations_node = scoped_tree_node(typesafe_sprintf("%x locations###locations", using_locations.size()).c_str())) {
						for (const auto& l : using_locations) {
							text(l);
						}
					}
				}
				else {
					text_disabled("(Nowhere)");
				}
			}

			ImGui::NextColumn();

			if (node && show_properties_column) {
				auto sc = scoped_indent();

				do_pathed_asset_properties(
					tree_settings,
					prop_in,
					cmd_in,
					path_entry, 
					ticked_in_range, 
					ticked_ids,
					is_current_ticked,
					game_atlas,
					preview,
					num_cols
				);
			}
		};

		int section_index = 0;
		
		auto do_section = [&](
			const auto& asset_entries,
			const std::array<std::string, 3> labels,
			const std::optional<rgba> color = std::nullopt
		) {
			if (asset_entries.empty()) {
				return;
			}

			official_separator = false;

			{
				do_tick_all_checkbox(
					settings,
					ticked_assets,
					[&asset_entries](auto callback) {
						for (const auto& p : asset_entries) {
							callback(p.id);
						}
					},
					section_index++
				);

				if (color) {
					text_color(labels[0], *color);
				}
				else {
					text_disabled(labels[0]);
				}
			}

			if (show_properties_column) {
				ImGui::NextColumn();
				text_disabled("Properties");
			}

			ImGui::NextColumn();
			text_disabled(labels[1]);
			ImGui::NextColumn();

			if (show_using_locations) {
				text_disabled(labels[2]);
				ImGui::NextColumn();
			}

			ImGui::Separator();

			const auto ticked_and_existing = get_all_ticked_and_existing(asset_entries);

			thread_local std::vector<asset_id_type> ticked_ids;
			ticked_ids.clear();

			for (const auto& p : ticked_and_existing) {
				ticked_ids.push_back(p.id);
			}

			for (const auto& p : asset_entries) {
				do_asset_entry(p, ticked_and_existing, ticked_ids);
			}

			ImGui::Separator();
		};

		if (path_browser_settings.show_orphaned) {
			do_section(
				missing_orphaned_paths,
				{ "Missing paths (orphaned)", "Last seen in", "Used at" },
				red
			);
		}

		do_section(
			missing_paths,
			{ "Missing paths", "Last seen in", "Used at" },
			red
		);

		if (path_browser_settings.show_orphaned) {
			do_section(
				orphaned_paths,
				{ typesafe_sprintf("Orphaned %xs", label), "Folder", "Used at" }
			);
		}

		do_section(
			used_paths,
			{ capitalize_first(std::string(label)) + "s", "Folder", "Used at" }
		);
	}
	else {

	}
#else
	(void)settings;
	(void)game_atlas;
	(void)cmd_in;
	(void)window;
#endif
}

template struct debugger_pathed_asset_gui<assets::image_id>;
template struct debugger_pathed_asset_gui<assets::sound_id>;
