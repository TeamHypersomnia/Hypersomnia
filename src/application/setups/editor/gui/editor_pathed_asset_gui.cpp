#define INCLUDE_TYPES_IN 1

#include "augs/string/string_templates.h"

#include "augs/templates/introspection_utils/field_name_tracker.h"
#include "augs/templates/introspection_utils/introspect_with_containers.h"

#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/path_tree_structs.h"

#include "game/organization/for_each_entity_type.h"

#include "application/setups/editor/gui/editor_pathed_asset_gui.h"
#include "application/setups/editor/editor_folder.h"
#include "application/intercosm.h"

#include "application/setups/editor/detail/other_styles.h"
#include "application/setups/editor/detail/format_struct_name.h"
#include "application/setups/editor/property_editor/widgets/asset_path_chooser.h"
#include "application/setups/editor/property_editor/general_edit_properties.h"
#include "application/setups/editor/detail/find_locations_that_use.h"
#include "application/setups/editor/detail/checkbox_selection.h"
#include "application/setups/editor/property_editor/compare_all_fields_to.h"
#include "application/setups/editor/property_editor/special_widgets.h"
#include "augs/readwrite/byte_readwrite.h"

#include "view/try_load_meta_lua.h"
#include "augs/templates/list_utils.h"

template <class id_type>
struct pathed_asset_entry : public browsed_path_entry_base<id_type> {
	using base = browsed_path_entry_base<id_type>;

	id_type id;
	std::vector<std::string> using_locations;

	bool missing = false;

	bool used() const {
		return using_locations.size() > 0;
	}

	pathed_asset_entry() = default;
	pathed_asset_entry(
		const maybe_official_path<id_type>& from,
	   	const id_type id
	) :
		base(from),
		id(id)
	{}
};

struct source_path_widget {
	all_viewables_defs& defs;
	const augs::path_type& project_path;
	const property_editor_settings& settings;
	const bool disabled;

	template <class T>
	static constexpr bool handles = is_maybe_official_path<T>::value;

	template <class T>
	auto describe_changed(
		const std::string& /* formatted_label */,
		const T& to
	) const {
		return typesafe_sprintf("Changed %x path to %x", to.get_label(), to.path);
	}

	template <class T>
	auto get_sane_default() const {
		return T();
	}

	template <class T>
	std::optional<tweaker_type> handle(const std::string& identity_label, T& object) const {
		using id_type = typename T::id_type;
		auto& definitions = get_viewable_pool<id_type>(defs);

		bool modified = false;

		auto scope = ::maybe_disabled_cols(settings, disabled);

		thread_local asset_path_chooser<id_type> chooser;

		chooser.perform(
			identity_label,
			object,
			project_path,
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

		if (modified) {
			return tweaker_type::DISCRETE;
		}

		return std::nullopt;
	}
};

template <class asset_id_type>
void editor_pathed_asset_gui<asset_id_type>::perform(
	const property_editor_settings& settings,
   	editor_command_input cmd_in
) {
	using namespace augs::imgui;

	auto window = base::make_scoped_window();

	if (!window) {
		return;
	}

	auto& folder = cmd_in.folder;
	auto& work = *folder.work;

	auto& viewables = work.viewables;

	using asset_entry_type = pathed_asset_entry<asset_id_type>;

	thread_local std::unordered_set<augs::path_type> _last_seen_missing_paths;
	/* Linker error fix */
	auto& last_seen_missing_paths = _last_seen_missing_paths;
	thread_local std::vector<asset_entry_type> missing_orphaned_paths;
	thread_local std::vector<asset_entry_type> missing_paths;

	thread_local std::vector<asset_entry_type> orphaned_paths;
	thread_local std::vector<asset_entry_type> used_paths;

	missing_orphaned_paths.clear();
	missing_paths.clear();

	orphaned_paths.clear();
	used_paths.clear();

	auto is_ticked = [this](const auto& p) {
		return found_in(ticked_assets, p.id);
	};

	auto get_all_ticked_and_existing = [&](auto in_range) {
		erase_if(in_range, [&] (const auto& candidate) { return !is_ticked(candidate); } );
		return in_range;
	};


	if (base::acquire_once) {
		acquire_missing_paths = true;
	}

	if (acquire_missing_paths) {
		last_seen_missing_paths.clear();
	}

	auto& definitions = get_viewable_pool<asset_id_type>(viewables);

	const auto label = std::string(maybe_official_path<asset_id_type>::get_label());

	for_each_id_and_object(definitions,
		[&](const auto id, const auto& object) mutable {
			const auto path = object.get_source_path();
			auto new_entry = asset_entry_type(path, id);

			find_locations_that_use(id, work, [&](const auto& location) {
				new_entry.using_locations.push_back(location);
			});

			auto push_missing = [&]() {
				(new_entry.used() ? missing_paths : missing_orphaned_paths).emplace_back(std::move(new_entry));
			};

			auto push_existing = [&]() {
				(new_entry.used() ? used_paths : orphaned_paths).emplace_back(std::move(new_entry));
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

			using def_type = std::remove_reference_t<decltype(object)>;
			const auto& view = asset_definition_view<def_type>(folder.current_path, object);

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

	if (ImGui::Button("Re-check existence")) {
		acquire_missing_paths = true;
	}

	ImGui::Separator();

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	base::acquire_keyboard_once();

	auto& tree_settings = path_browser_settings.tree_settings;

	auto for_each_range = [](auto callback) {
		callback(missing_orphaned_paths);
		callback(missing_paths);

		callback(orphaned_paths);
		callback(used_paths);
	};

	auto prepare = [&](auto& range){
		erase_if(range, [&](const auto& entry){
			const auto displayed_name = tree_settings.get_prettified(entry.get_filename());
			const auto displayed_dir = entry.get_displayed_directory();

			if (!filter.PassFilter(displayed_name.c_str()) && !filter.PassFilter(displayed_dir.c_str())) {
				return true;
			}

			return false;
		});

		sort_range(range);
	};

	for_each_range(prepare);

	auto files_view = scoped_child("Files view");

	const auto num_cols = 4;

	if (tree_settings.linear_view) {
		ImGui::Columns(num_cols);

		bool official_separator = false;

		auto do_path = [&](const auto& path_entry, const auto& ticked_in_range, const auto& ticked_ids) {
			separator_if_unofficials_ended(path_entry.get_full_path(), official_separator);

			const auto id = path_entry.id;
			auto scope = scoped_id(id);

			const auto displayed_name = tree_settings.get_prettified(path_entry.get_filename());
			const auto displayed_dir = path_entry.get_displayed_directory();

			const auto current_ticked = is_ticked(path_entry);

			const auto flags = do_selection_checkbox(ticked_assets, id, current_ticked, id);

			if (!path_entry.used()) {
				const auto scoped_style = in_line_button_style();

				if (ImGui::Button("F")) {
					auto forget = [&](const auto& which, const bool has_parent) {
						forget_asset_id_command<asset_id_type> cmd;
						cmd.freed_id = which.id;
						cmd.built_description = 
							typesafe_sprintf("Stopped tracking %x", which.get_full_path().to_display())
						;

						cmd.common.has_parent = has_parent;
						post_editor_command(cmd_in, std::move(cmd));
					};

					if (!current_ticked) {
						forget(path_entry, false);
					}
					else {
						const auto& all = ticked_in_range;

						forget(all[0], false);

						for (std::size_t i = 1; i < all.size(); ++i) {
							forget(all[i], true);
						}
					}
				}

				if (ImGui::IsItemHovered()) {
					if (current_ticked && ticked_in_range.size() > 1) {
						text_tooltip("Forget %x %xs", ticked_in_range.size(), label);
					}
					else {
						text_tooltip("Forget %x", path_entry.get_full_path().to_display());
					}
				}

				ImGui::SameLine();
			}

			if (nullptr == mapped_or_nullptr(definitions, id)) {
				/* It has just been forgotten. */
				return;
			}

			const auto node = scoped_tree_node_ex(displayed_name + "###Node", flags);

			next_columns(2);

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
				text_disabled("(Nowhere)");
			}

			ImGui::NextColumn();

			if (node) {
				auto& definition_object = definitions[id];
				const auto& project_path = cmd_in.folder.current_path;
				using command_type = change_asset_property_command<asset_id_type>;

				auto sc = scoped_indent();

				{
					using def_type = std::remove_reference_t<decltype(definition_object)>;
					const auto& view = asset_definition_view<def_type>(project_path, definition_object);
					const auto resolved = view.get_resolved_source_path();

					{
						const auto meta_lua_path = get_meta_lua_path(resolved);
						auto cols = maybe_disabled_cols(settings, !augs::exists(meta_lua_path));

						if (ImGui::Button("Read defaults")) {
							try {
								decltype(definition_object.meta) new_meta;
								try_load_meta_lua(cmd_in.lua, new_meta, resolved);

								command_type cmd;

								cmd.affected_assets = { id };
								cmd.property_id.field = make_field_address(definition_object, definition_object.meta);
								cmd.value_after_change = augs::to_bytes(new_meta);
								cmd.built_description = "Read defaults from " + augs::to_display(meta_lua_path);

								post_editor_command(cmd_in, std::move(cmd));
							}
							catch (...) {

							}
						}

						ImGui::SameLine();
					}

					{
						if (ImGui::Button("Write defaults")) {
							save_meta_lua(cmd_in.lua, definition_object.meta, resolved);
						}
					}

					next_columns(num_cols);
				}

				const auto property_location = typesafe_sprintf(" (in %x)", displayed_name);

				auto post_new_change = [&](
					const auto& description,
					const auto field_id,
					const auto& new_content
				) {
					command_type cmd;

					if constexpr(source_path_widget::handles<remove_cref<decltype(new_content)>>) {
						/* 
							If we are changing the source path inside an asset that is ticked,
							don't propagate the change throughout the rest of ticks
							because we uniquely identify exactly by the source path.
						*/

						cmd.affected_assets = { id };
					}
					else {
						if (current_ticked) {
							cmd.affected_assets = ticked_ids;
						}
						else {
							cmd.affected_assets = { id };
						}
					}

					cmd.property_id.field = field_id;
					cmd.value_after_change = augs::to_bytes(new_content);
					cmd.built_description = description + property_location;

					post_editor_command(cmd_in, std::move(cmd));
				};

				auto rewrite_last_change = [&](
					const auto& description,
					const auto& new_content
				) {
					auto& last = cmd_in.folder.history.last_command();

					if (auto* const cmd = std::get_if<command_type>(std::addressof(last))) {
						cmd->built_description = description + property_location;
						cmd->rewrite_change(augs::to_bytes(new_content), cmd_in);
					}
					else {
						LOG("WARNING! There was some problem with tracking activity of editor controls.");
					}
				};

				auto prop_in = property_editor_input { settings, property_editor_data };

				const bool disable_path_chooser = current_ticked && ticked_ids.size() > 1;

				general_edit_properties(
					prop_in,
					definition_object,
					post_new_change,
					rewrite_last_change,
					[&](const auto& first, const field_address field_id) {
						if (!current_ticked) {
							return true;
						}

						return compare_all_fields_to(
							first,
							asset_property_id<asset_id_type> { field_id }, 
							cmd_in, 
							ticked_ids
						);
					},
					special_widgets(
						source_path_widget { viewables, project_path, settings, disable_path_chooser }
					),
					default_sane_default_provider(),
					num_cols - 2
				);
			}
		};

		int s = 0;
		
		auto do_section = [&](
			const auto& paths,
			const std::array<std::string, 3> labels,
			const std::optional<rgba> color = std::nullopt
		) {
			if (paths.empty()) {
				return;
			}

			official_separator = false;

			{
				do_tick_all_checkbox(
					settings,
					ticked_assets,
					[&paths](auto callback) {
						for (const auto& p : paths) {
							callback(p.id);
						}
					},
					s++
				);

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

			const auto ticked_and_existing = get_all_ticked_and_existing(paths);

			thread_local std::vector<asset_id_type> ticked_ids;
			ticked_ids.clear();

			for (const auto& p : ticked_and_existing) {
				ticked_ids.push_back(p.id);
			}

			for (const auto& p : paths) {
				do_path(p, ticked_and_existing, ticked_ids);
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
}

template struct editor_pathed_asset_gui<assets::image_id>;
template struct editor_pathed_asset_gui<assets::sound_id>;
