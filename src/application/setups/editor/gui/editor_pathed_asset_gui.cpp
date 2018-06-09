#define INCLUDE_TYPES_IN 1

#include "augs/string/string_templates.h"

#include "augs/templates/introspection_utils/field_name_tracker.h"
#include "augs/templates/introspection_utils/introspect_with_containers.h"

#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_game_image.h"
#include "augs/misc/imgui/path_tree_structs.h"
#include "augs/misc/imgui/imgui_drawers.h"

#include "game/organization/for_each_entity_type.h"
#include "view/viewables/images_in_atlas_map.h"

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

inline std::optional<rgba> image_color_picker(
	const int zoom,
	const augs::atlas_entry& entry,
	const augs::image& viewed_image
) {
	using namespace augs::imgui;

	auto& io = ImGui::GetIO();

	const auto is = vec2i(entry.get_original_size());

	const auto viewing_size = (is * zoom).operator ImVec2();

	text("Image size: %x, zoom: %x", is, zoom);

	invisible_button_reset_cursor("###OffsetSelector", viewing_size);
	game_image(entry, viewing_size);

	const auto cross_alpha = 200;

	auto draw_cross = [is, zoom](const vec2i where, const rgba col) {
		draw_cross_overlay(is, where, zoom, col);
	};

	const auto pos = ImGui::GetCursorScreenPos();

	const auto image_space_new = vec2i(vec2(io.MousePos.x - pos.x, io.MousePos.y - pos.y) / zoom);

	std::optional<rgba> result;

	if (ImGui::IsItemClicked()) {
		result = viewed_image.pixel(image_space_new);
	}

	if (ImGui::IsItemHovered()) {
		draw_cross(image_space_new, rgba(green.rgb(), cross_alpha));

		auto scope = scoped_tooltip();
		text("Image space: %x", image_space_new);
		rgba color_preview = viewed_image.pixel(image_space_new);
		color_edit("##colorpreview", color_preview);
	}

	return result;
}

struct image_color_picker_widget {
	const assets::image_id id;
	const images_in_atlas_map& game_atlas;
	const image_definitions_map& defs;
	editor_image_preview& current_preview;
	const augs::path_type& project_path;

	template <class T>
	static constexpr bool handles = is_one_of_v<T, rgba>;

	template <class T>
	auto describe_changed(
		const std::string& formatted_label,
		const T& to
	) const {
		return typesafe_sprintf("Changed %x to %x", formatted_label, to);
	}

	void update_preview() const {
		const auto& p = defs[id].get_source_path();

		if (current_preview.path != p) {
			current_preview.image.from_file(p.resolve(project_path));
			current_preview.path = p;
		}
	}

	template <class T>
	bool handle_container_prologue(const std::string& identity_label, T& object) const {
		using namespace augs::imgui;

		static_assert(handles<typename T::value_type>);

		update_preview();

		if (auto combo = scoped_combo(("###AddMultiple" + identity_label).c_str(), "Add multiple...", ImGuiComboFlags_HeightLargest)) {
			const auto zoom = 4;
			const auto& entry = game_atlas.at(id).diffuse;

			if (const auto picked = image_color_picker(zoom, entry, current_preview.image)) {
				object.emplace_back(*picked);
				return true;
			}
		}

		return false;
	}

	template <class T>
	auto handle(const std::string& identity_label, T& object) const {
		using namespace augs::imgui;

		std::optional<tweaker_type> result;

		update_preview();

		auto iw = scoped_item_width(60);

		if (auto combo = scoped_combo((identity_label + "Picker").c_str(), "Pick", ImGuiComboFlags_HeightLargest)) {
			const auto zoom = 4;
			const auto& entry = game_atlas.at(id).diffuse;

			if (const auto picked = image_color_picker(zoom, entry, current_preview.image)) {
				object = *picked;
				result = tweaker_type::DISCRETE;
			}
		}

		if (result) {
			return result;
		}

		/* Perform the standard widget for manual tweaking */

		{
			ImGui::SameLine();

			auto iw = scoped_item_width(-1);

			if (color_edit(identity_label, object)) {
				result = tweaker_type::CONTINUOUS;
			}
		}

		return result;
	}
};

struct image_offset_widget {
	const assets::image_id id;
	const images_in_atlas_map& game_atlas;

	template <class T>
	static constexpr bool handles = is_one_of_v<T, vec2i, transformi>;

	template <class T>
	auto describe_changed(
		const std::string& formatted_label,
		const T& to
	) const {
		return typesafe_sprintf("Changed %x to %x", formatted_label, to);
	}

	template <class T>
	auto handle(const std::string& identity_label, T& object) const {
		using namespace augs::imgui;

		constexpr bool only_vec = std::is_same_v<T, vec2i>;

		auto& current_pos = [&]() -> vec2i& {
			if constexpr(only_vec) {
				return object;
			}
			else {
				return object.pos;
			}
		}();

		std::optional<tweaker_type> result;

		auto iw = scoped_item_width(80);

		if (auto combo = scoped_combo((identity_label + "Picker").c_str(), "Pick...", ImGuiComboFlags_HeightLargest)) {
            auto& io = ImGui::GetIO();

			const auto& entry = game_atlas.at(id);

			const auto is = vec2i(entry.get_original_size());
			const auto zoom = 4;

			const auto viewing_size = (is * zoom).operator ImVec2();

			text("Image size: %x, zoom: %x", is, zoom);

			invisible_button_reset_cursor("###OffsetSelector", viewing_size);
			game_image(entry.diffuse, viewing_size);

			const auto cross_alpha = 200;
			const auto ray_alpha = 120;

			auto draw_ray = [is](const vec2i center, const int degrees, const rgba col) {
				draw_ray_overlay(is, center, degrees, zoom, col);
			};

			auto draw_cross = [is](const vec2i where, const rgba col) {
				draw_cross_overlay(is, where, zoom, col);
			};

			const bool reference_to_the_right = identity_label == "##bullet_spawn";

			const auto reference_point = 
				reference_to_the_right ? 
				vec2i(is.x - 1, is.y / 2) 
				: vec2i(is / 2)
			;

			const auto pos = ImGui::GetCursorScreenPos();

			const auto image_space_new = vec2i(vec2(io.MousePos.x - pos.x, io.MousePos.y - pos.y) / zoom);
			const auto image_space_old = reference_point + current_pos;

			const auto chosen_new_offset = image_space_new - reference_point;

			draw_cross(image_space_old, rgba(red.rgb(), cross_alpha));

			if constexpr(!only_vec) {
				draw_ray(image_space_old, object.rotation, rgba(white.rgb(), ray_alpha));
			}

			const bool pos_mode = only_vec || !io.KeyShift;

			if (pos_mode) {
				if (ImGui::IsItemClicked()) {
					current_pos = chosen_new_offset;
					result = tweaker_type::DISCRETE;
				}

				if (ImGui::IsItemHovered()) {
					draw_cross(image_space_new, rgba(green.rgb(), cross_alpha));

					text_tooltip("Chosen offset: %x\nImage space: %x", chosen_new_offset, image_space_new);
				}
			}
			else {
				if constexpr(!only_vec) {
					const auto degrees_new = (image_space_new - image_space_old).degrees();

					if (ImGui::IsItemClicked()) {
						object.rotation = degrees_new;
						result = tweaker_type::DISCRETE;
					}

					if (ImGui::IsItemHovered()) {
						draw_cross(image_space_new, rgba(green.rgb(), ray_alpha));
						draw_ray(image_space_old, degrees_new, rgba(green.rgb(), cross_alpha));

						text_tooltip("Chosen rotation: %x", degrees_new);
					}
				}
			}
		}

		if (result) {
			return result;
		}

		/* Perform the standard widget for manual tweaking */
		{
			ImGui::SameLine();

			auto iw = scoped_item_width(-1);

			if constexpr(only_vec) {
				if (drag_vec2(identity_label, object)) {
					result = tweaker_type::CONTINUOUS;
				}
			}
			else {
				if (drag_transform(identity_label, object)) {
					result = tweaker_type::CONTINUOUS;
				}
			}
		}

		return result;
	}
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
	const images_in_atlas_map& game_atlas,
   	editor_command_input cmd_in
) {
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
}

template struct editor_pathed_asset_gui<assets::image_id>;
template struct editor_pathed_asset_gui<assets::sound_id>;
