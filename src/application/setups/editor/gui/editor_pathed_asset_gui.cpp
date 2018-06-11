#define INCLUDE_TYPES_IN 1

#include "application/setups/editor/gui/editor_pathed_asset_gui.h"

#if BUILD_PROPERTY_EDITOR
#include "augs/string/string_templates.h"

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

#include "application/setups/editor/editor_folder.h"
#include "application/intercosm.h"

#include "application/setups/editor/detail/other_styles.h"
#include "application/setups/editor/detail/format_struct_name.h"

#include "application/setups/editor/property_editor/widgets/asset_path_chooser.h"
#include "application/setups/editor/property_editor/widgets/image_color_picker_widget.h"
#include "application/setups/editor/property_editor/widgets/image_offset_widget.h"
#include "application/setups/editor/property_editor/widgets/source_path_widget.h"

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

#endif

template <class asset_id_type>
void editor_pathed_asset_gui<asset_id_type>::perform(
	const property_editor_settings& settings,
	const images_in_atlas_map& game_atlas,
   	editor_command_input cmd_in
) {
#if BUILD_PROPERTY_EDITOR
	constexpr bool is_image_type = std::is_same_v<asset_id_type, assets::image_id>;

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
	using def_type = typename std::remove_reference_t<decltype(definitions)>::mapped_type;

	const auto label = std::string(maybe_official_path<asset_id_type>::get_label());

	for_each_id_and_object(definitions,
		[&](const asset_id_type id, const def_type& object) mutable {
			const auto path = object.get_source_path();
			auto new_entry = asset_entry_type(path, id);

			find_locations_that_use(id, work, [&](const std::string& location) {
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

	const bool show_using_locations = path_browser_settings.show_using_locations;
	const auto num_cols = 3 + (show_using_locations ? 1 : 0);

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

			if constexpr(is_image_type) {
				if (ImGui::IsItemHovered()) {
					auto tooltip = scoped_tooltip();
					game_image_button("###TooltipPreview", game_atlas.at(id).diffuse);
				}
			}

			next_columns(2);

			text_disabled(displayed_dir);

			if (show_using_locations) {
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
			}

			ImGui::NextColumn();

			if (node) {
				auto& definition_object = definitions[id];
				const auto& project_path = cmd_in.folder.current_path;
				using command_type = change_asset_property_command<asset_id_type>;

				auto sc = scoped_indent();

				{
					using def_type = std::remove_reference_t<decltype(definition_object)>;
					const auto& view = asset_definition_view<const def_type>(project_path, definition_object);
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

						if (ImGui::IsItemHovered()) {
							text_tooltip("Read defaults from:\n%x", meta_lua_path);
						}

						ImGui::SameLine();
					}

					{
						if (ImGui::Button("Write defaults")) {
							if (!current_ticked) {
								save_meta_lua(cmd_in.lua, definition_object.meta, resolved);
							}
							else {
								for (const auto& t : ticked_in_range) {
									const auto& ticked_definition_object = definitions[t.id];
									const auto& ticked_view = asset_definition_view<const def_type>(project_path, ticked_definition_object);
									const auto ticked_resolved = ticked_view.get_resolved_source_path();

									save_meta_lua(cmd_in.lua, ticked_definition_object.meta, ticked_resolved);
								}
							}
						}

						if (ImGui::IsItemHovered()) {
							auto get_lwt = [](const augs::path_type& meta_path) {
								try {
									const auto lwt = augs::last_write_time(meta_path);
									return typesafe_sprintf("(LM: %x)", augs::date_time(lwt).how_long_ago_tell_seconds());
								}
								catch (...) {
									return std::string("(Doesn't exist)");
								}
							};

							auto print_path = [&](const augs::path_type& ticked_resolved) {
								const auto meta_path = get_meta_lua_path(ticked_resolved);
								return meta_path.string() + " " + get_lwt(meta_path);
							};

							if (!current_ticked) {
								text_tooltip("Write defaults to:\n%x", print_path(resolved));
							}
							else {
								std::string all_resolved;

								for (const auto& t : ticked_in_range) {
									const auto& ticked_definition_object = definitions[t.id];
									const auto& ticked_view = asset_definition_view<const def_type>(project_path, ticked_definition_object);
									const auto ticked_resolved = ticked_view.get_resolved_source_path();

									all_resolved += print_path(ticked_resolved) + "\n";
								}

								text_tooltip("Write defaults to:\n%x", all_resolved);
							}
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

				/* 
					Don't construct this widget for sounds and other pathed widgets. 
					Image offsets apply only to... images.
				*/

				using offset_widget = 
					std::conditional_t<
						is_image_type,
						image_offset_widget,
					   	default_widget_provider
					>
				;

				using color_widget =
					std::conditional_t<
						is_image_type,
						image_color_picker_widget,
					   	default_widget_provider
					>
				;

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
						source_path_widget { viewables, project_path, settings, disable_path_chooser },
						offset_widget { id, game_atlas },
						color_widget { id, game_atlas, viewables.image_definitions, preview, project_path }
					),
					default_sane_default_provider(),
					num_cols - 2
				);
			}
		};

		int section_index = 0;
		
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
					section_index++
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

			if (show_using_locations) {
				text_disabled(labels[2]);
				ImGui::NextColumn();
			}

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
#else
	(void)settings;
	(void)game_atlas;
	(void)cmd_in;
#endif
}

template struct editor_pathed_asset_gui<assets::image_id>;
template struct editor_pathed_asset_gui<assets::sound_id>;
