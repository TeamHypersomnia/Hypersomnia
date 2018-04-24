#define INCLUDE_TYPES_IN 1

#include "augs/string/string_templates.h"

#include "augs/templates/introspection_utils/field_name_tracker.h"
#include "augs/templates/introspection_utils/introspect_with_containers.h"

#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/path_tree_structs.h"

#include "game/organization/for_each_entity_type.h"

#include "application/setups/editor/gui/editor_assets_gui.h"
#include "application/setups/editor/editor_folder.h"
#include "application/intercosm.h"

#include "application/setups/editor/detail/format_struct_name.h"
#include "application/setups/editor/property_editor/asset_control_provider.h"
#include "application/setups/editor/property_editor/general_edit_properties.h"
#include "application/setups/editor/detail/find_locations_that_use.h"
#include "application/setups/editor/property_editor/compare_all_fields_to.h"

#include "augs/templates/list_utils.h"

template <class id_type>
struct asset_gui_path_entry : public browsed_path_entry_base {
	using base = browsed_path_entry_base;

	id_type id;
	std::vector<std::string> using_locations;

	bool missing = false;

	bool used() const {
		return using_locations.size() > 0;
	}

	asset_gui_path_entry() = default;
	asset_gui_path_entry(
		const maybe_official_path& from,
	   	const id_type id
	) :
		id(id),
		base(from)
	{}
};

template <class id_type>
struct path_chooser_provider {
	all_viewables_defs& defs;
	const augs::path_type& project_path;
	const property_editor_settings& settings;
	const bool disabled;

	using handled_type = maybe_official_path;

	template <class T>
	static constexpr bool handles = std::is_same_v<T, handled_type>;

	auto describe_changed(
		const std::string& label,
		const handled_type& from,
		const handled_type& to
	) const {
		return description_pair {
			"",
			typesafe_sprintf("Changed image path from %x", describe_moved_file(from.path, to.path))
		};
	}

	bool handle(const std::string& label, handled_type& object, const field_address& address) const {
		auto& definitions = defs.image_definitions;

		bool modified = false;

		auto scope = ::maybe_disabled_cols(settings, disabled);

		choose_asset_path(
			"##Source-path",
			object,
			project_path,
			"gfx",
			[&](const auto& chosen_path) {
				object = chosen_path;
				modified = true;
			},
			[&](const auto& candidate_path) {
				if (const auto asset_id = ::find_asset_id_by_path(candidate_path, definitions)) {
					return false;
				}

				return true;
			},
			"Already tracked paths"
		);

		return modified;
	}
};

void editor_images_gui::perform(
	const property_editor_settings& settings,
   	editor_command_input cmd_in
) {
	using namespace augs::imgui;

	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	auto& folder = cmd_in.folder;
	auto& work = *folder.work;

	auto& viewables = work.viewables;

	using asset_id_type = assets::image_id;
	using path_entry_type = asset_gui_path_entry<asset_id_type>;

	thread_local std::unordered_set<augs::path_type> last_seen_missing_paths;

	thread_local std::vector<path_entry_type> missing_orphaned_paths;
	thread_local std::vector<path_entry_type> missing_paths;

	thread_local std::vector<path_entry_type> orphaned_paths;
	thread_local std::vector<path_entry_type> used_paths;

	auto is_selected = [this](const auto& p) {
		return found_in(selected_images, p.id);
	};

	auto get_all_selected_and_existing = [&](auto in_range) {
		erase_if(in_range, [&] (const auto& candidate) { return !is_selected(candidate); } );
		return in_range;
	};

	missing_orphaned_paths.clear();
	missing_paths.clear();

	orphaned_paths.clear();
	used_paths.clear();

	if (acquire_once) {
		acquire_missing_paths = true;
	}

	if (acquire_missing_paths) {
		last_seen_missing_paths.clear();
	}

	auto& definitions = viewables.image_definitions;

	definitions.for_each_object_and_id(
		[&](const auto& object, const auto id) mutable {
			const auto path = object.get_source_path();
			auto new_entry = path_entry_type(path, id);

			const auto& view = image_definition_view(folder.current_path / "gfx", object);

			find_locations_that_use(id, work, [&](const auto& location) {
				new_entry.using_locations.push_back(location);
			});

			auto push_missing = [&]() {
				(new_entry.used() ? missing_paths : missing_orphaned_paths).emplace_back(std::move(new_entry));
			};

			auto push_existing = [&]() {
				(new_entry.used() ? used_paths : orphaned_paths).emplace_back(std::move(new_entry));
			};

			auto lazy_check_missing = [this](const auto& p) {
				if (acquire_missing_paths) {
					if (!augs::exists(p)) {
						last_seen_missing_paths.emplace(p);
						return true;
					}

					return false;
				}

				return found_in(last_seen_missing_paths, p);
			};

			if (lazy_check_missing(view.get_source_image_path())) {
				push_missing();
			}
			else {
				push_existing();
			}
		}
	);

	acquire_missing_paths = false;
	
	browser_settings.do_tweakers();

	if (ImGui::Button("Re-check existence")) {
		acquire_missing_paths = true;
	}

	ImGui::Separator();

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	acquire_keyboard_once();

	auto& tree_settings = browser_settings.tree_settings;

	auto for_each_range = [](auto callback) {
		callback(missing_orphaned_paths);
		callback(missing_paths);

		callback(orphaned_paths);
		callback(used_paths);
	};

	auto prepare_range = [&](auto& r){
		erase_if(r, [&](const auto& entry){
			const auto displayed_name = tree_settings.get_prettified(entry.get_filename());
			const auto displayed_dir = entry.get_displayed_directory();

			if (!filter.PassFilter(displayed_name.c_str()) && !filter.PassFilter(displayed_dir.c_str())) {
				return true;
			}

			return false;
		});

		sort_range(r);
	};

	for_each_range(prepare_range);

	auto files_view = scoped_child("Files view");

	auto& history = cmd_in.folder.history;

	const auto num_cols = 4;

	if (tree_settings.linear_view) {
		ImGui::Columns(num_cols);

		int i = 0;

		auto do_path = [&](const auto& path_entry, const auto& selected_in_range, const auto& selected_ids) {
			auto scope = scoped_id(i++);

			const auto id = path_entry.id;

			const auto displayed_name = tree_settings.get_prettified(path_entry.get_filename());
			const auto displayed_dir = path_entry.get_displayed_directory();

			auto scoped_style = scoped_style_var(ImGuiStyleVar_ItemSpacing, ImVec2(3, 1));
			auto scoped_style2 = scoped_style_var(ImGuiStyleVar_FramePadding, ImVec2(1, 1));

			const auto current_selected = is_selected(path_entry);

			{
				bool altered = current_selected;

				if (checkbox(typesafe_sprintf("###%x", i).c_str(), altered)) {
					if (current_selected && !altered) {
						erase_element(selected_images, id);
					}
					else if(!current_selected && altered) {
						selected_images.emplace(id);
					}
				}
			}

			ImGuiTreeNodeFlags flags = 0;

			if (current_selected) {
				flags = ImGuiTreeNodeFlags_Selected;
			}

			ImGui::SameLine();
			const auto node = scoped_tree_node_ex(displayed_name.c_str(), flags);

			scoped_style.finish_scope();
			scoped_style2.finish_scope();

			ImGui::NextColumn();
			ImGui::NextColumn();

			const auto& project_path = cmd_in.folder.current_path;

			text_disabled(displayed_dir);

			ImGui::NextColumn();

			if (path_entry.used()) {
				const auto& using_locations = path_entry.using_locations;

				if (auto node = scoped_tree_node(typesafe_sprintf("%x locations###locations", using_locations.size()).c_str())) {
					for (const auto& l : using_locations) {
						text(l);
					}
				}
			}
			else {
				const auto scoped_style = scoped_style_var(ImGuiStyleVar_FramePadding, ImVec2(3, 0));

				if (ImGui::Button("Forget")) {
					auto forget = [&](const auto& which, const bool has_parent) {
						forget_asset_id_command<asset_id_type> cmd;
						cmd.forgotten_id = which.id;
						cmd.built_description = 
							typesafe_sprintf("Stopped tracking %x", which.get_full_path().to_display())
						;

						cmd.common.has_parent = has_parent;
						history.execute_new(std::move(cmd), cmd_in);
					};

					if (current_selected) {
						forget(path_entry, false);
					}
					else {
						const auto& all = selected_in_range;

						forget(all[0], false);

						for (std::size_t i = 1; i < all.size(); ++i) {
							forget(all[i], true);
						}
					}
				}
			}

			ImGui::NextColumn();

			if (node) {
				auto sc = scoped_indent();

				const auto property_location = typesafe_sprintf(" (in %x)", displayed_name);

				auto& history = cmd_in.folder.history;
				auto& defs = cmd_in.folder.work->viewables;

				using command_type = change_asset_property_command<asset_id_type>;

				auto post_new_change = [&](
					const auto& description,
					const auto field_id,
					const auto& new_content
				) {
					command_type cmd;

					if constexpr(std::is_same_v<decltype(new_content), const maybe_official_path&>) {
						cmd.affected_assets = { id };
					}
					else {
						if (current_selected) {
							cmd.affected_assets = selected_ids;
						}
						else {
							cmd.affected_assets = { id };
						}
					}

					cmd.property_id.field = field_id;
					cmd.value_after_change = augs::to_bytes(new_content);
					cmd.built_description = description + property_location;

					history.execute_new(std::move(cmd), cmd_in);
				};

				auto rewrite_last_change = [&](
					const auto& description,
					const auto& new_content
				) {
					auto& last = history.last_command();

					if (auto* const cmd = std::get_if<command_type>(std::addressof(last))) {
						cmd->built_description = description + property_location;
						cmd->rewrite_change(augs::to_bytes(new_content), cmd_in);
					}
					else {
						LOG("WARNING! There was some problem with tracking activity of editor controls.");
					}
				};

				const auto project_path = cmd_in.folder.current_path;

				auto prop_in = property_editor_input { settings, property_editor_data };

				const bool disable_path_chooser = current_selected && selected_ids.size() > 1;

				general_edit_properties(
					prop_in,
					definitions[id],
					post_new_change,
					rewrite_last_change,
					[&](const auto& first, const field_address field_id) {
						if (!current_selected) {
							return true;
						}

						return compare_all_fields_to(
							first,
							asset_property_id<asset_id_type> { field_id }, 
							viewables, 
							selected_ids
						);
					},
					path_chooser_provider<asset_id_type> { viewables, project_path, settings, disable_path_chooser },
					num_cols - 2
				);
			}
		};

		auto do_section = [&](
			const auto& paths,
			const std::array<std::string, 3> labels,
			const std::optional<rgba> color = std::nullopt
		) {
			if (paths.empty()) {
				return;
			}

			{
				bool all_selected = true;
				bool any_selected = false;

				for (const auto& e : paths) {
					if (is_selected(e)) {
						any_selected = true;
					}
					else {
						all_selected = false;
					}
				}

				const auto scoped_style = scoped_style_var(ImGuiStyleVar_ItemSpacing, ImVec2(3, 1));
				const auto scoped_style2 = scoped_style_var(ImGuiStyleVar_FramePadding, ImVec2(1, 1));

				{
					auto cols = ::maybe_different_value_cols(
						settings,
						any_selected && !all_selected
					);

					bool altered = all_selected;
					
					if (checkbox(typesafe_sprintf("###%x", i).c_str(), altered)) {
						if (all_selected && !altered) {
							for (const auto& e : paths) {
								erase_element(selected_images, e.id);
							}
						}
						else if (!all_selected && altered) {
							for (const auto& e : paths) {
								selected_images.insert(e.id);
							}
						}
					}
				}

				ImGui::SameLine();

				if (color) {
					text_color(labels[0], *color);
				}
				else {
					text_disabled(labels[0]);
				}
			}

			ImGui::NextColumn();
			text_disabled("Properties");

			ImGui::NextColumn();
			text_disabled(labels[1]);
			ImGui::NextColumn();
			text_disabled(labels[2]);
			ImGui::NextColumn();
			ImGui::Separator();

			const auto selected_and_existing = get_all_selected_and_existing(paths);

			thread_local std::vector<asset_id_type> selected_ids;
			selected_ids.clear();

			for (const auto& p : selected_and_existing) {
				selected_ids.push_back(p.id);
			}

			for (const auto& p : paths) {
				do_path(p, selected_and_existing, selected_ids);
			}

			ImGui::Separator();
		};

		if (browser_settings.show_orphaned) {
			do_section(
				missing_orphaned_paths,
				{ "Missing paths (orphaned)", "Last seen in", "Operations" },
				red
			);
		}

		do_section(
			missing_paths,
			{ "Missing paths", "Last seen in", "Used at" },
			red
		);

		if (browser_settings.show_orphaned) {
			do_section(
				orphaned_paths,
				{ "Orphaned images", "Folder", "Operations" }
			);
		}

		do_section(
			used_paths,
			{ "Images", "Folder", "Used at" }
		);
	}
	else {

	}
}
