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

#include "augs/templates/introspection_utils/types_in.h"
#include "augs/templates/list_utils.h"

#include "augs/templates/introspection_utils/find_object_in_object.h"

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

template <class T>
struct ignore_while_looking_for_id : std::bool_constant<
	is_one_of_v<T, all_logical_assets, all_entity_flavours>
> {};

template <class F>
void find_locations_that_use(
	const assets::image_id id,
	const intercosm& inter,
	F location_callback
) {
	auto traverse = [&](const std::string& preffix, const auto& object) {
		find_object_in_object<ignore_while_looking_for_id>(id, object, [&](const auto& location) {
			location_callback(preffix + location);
		});
	};

	const auto& common = inter.world.get_common_significant();

	traverse("Common: ", common);

	for_each_entity_type([&](auto e){ 
		using E = decltype(e);
		using Fl = entity_flavour<E>;

		if constexpr(can_type_contain_another_v<Fl, assets::image_id>) {
			common.get_flavours<E>().for_each([&](const auto, const auto& flavour) {
				const auto& name = flavour.template get<invariants::name>().name;

				auto for_each_through = [&](const auto& where) {
					for_each_through_std_get(
						where,
						[&](const auto& c) {
							using C = std::decay_t<decltype(c)>;

							if constexpr(can_type_contain_another_v<C, assets::image_id>) {
								find_object_in_object(id, c, [&](const auto& location) {
									/* location_callback(format_struct_name(c) + "of " + name + ": " + location); */
									location_callback("Flavour: " + name + " (" + format_struct_name(c) + "." + location + ")");
								});
							}
						}
					);
				};

				for_each_through(flavour.initial_components);
				for_each_through(flavour.invariants);
			});
		}
	});

	//traverse("Particle effects: ", inter.viewables.particle_effects);
}

void editor_images_gui::perform(editor_command_input in) {
	using namespace augs::imgui;

	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	auto& folder = in.folder;
	auto& work = *folder.work;

	auto& viewables = work.viewables;

	using asset_id_type = assets::image_id;
	using path_entry_type = asset_gui_path_entry<asset_id_type>;

	thread_local std::unordered_set<augs::path_type> last_seen_missing_paths;

	thread_local std::vector<path_entry_type> missing_orphaned_paths;
	thread_local std::vector<path_entry_type> missing_paths;

	thread_local std::vector<path_entry_type> orphaned_paths;
	thread_local std::vector<path_entry_type> used_paths;

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

	auto& loadables = viewables.image_loadables;

	loadables.for_each_object_and_id(
		[&](const auto& object, const auto id) mutable {
			const auto path = object.source_image;
			auto new_entry = path_entry_type(path, id);

			const auto& view = image_loadables_def_view(folder.current_path / "gfx", object);

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

	sort_range(missing_orphaned_paths);
	sort_range(missing_paths);

	sort_range(orphaned_paths);
	sort_range(used_paths);
	
	browser_settings.do_tweakers();

	if (ImGui::Button("Re-check existence")) {
		acquire_missing_paths = true;
	}

	ImGui::Separator();

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	acquire_keyboard_once();

	auto files_view = scoped_child("Files view");

	auto& tree_settings = browser_settings.tree_settings;

	auto& history = in.folder.history;

	if (tree_settings.linear_view) {
		ImGui::Columns(3);

		int i = 0;

		auto do_path = [&](const auto& path_entry) {
			auto scope = scoped_id(i++);

			const auto displayed_name = tree_settings.get_prettified(path_entry.get_filename());
			const auto displayed_dir = path_entry.get_displayed_directory();

			const auto id = path_entry.id;

			if (!filter.PassFilter(displayed_name.c_str()) && !filter.PassFilter(displayed_dir.c_str())) {
				return;
			}

			{
				const auto& project_path = in.folder.current_path;

				if (auto node = scoped_tree_node(displayed_name.c_str())) {
					choose_asset_path(
						"Source image",
						path_entry.get_full_path(),
						project_path,
						"gfx",
						[&](const auto& chosen_path) {
							auto& l = loadables[id];
							change_asset_property_command<assets::image_id> cmd;

							cmd.affected_assets = { id };

							cmd.built_description = typesafe_sprintf("Changed image path from %x", describe_moved_file(l.source_image.path, chosen_path.path));
							cmd.field = make_field_address(&image_loadables_def::source_image);
							cmd.value_after_change = augs::to_bytes(chosen_path);

							history.execute_new(std::move(cmd), in);
						},
						[&](const auto& candidate_path) {
							if (const auto asset_id = ::find_asset_id_by_path(candidate_path, loadables)) {
								return false;
							}

							return true;
						},
						"Already tracked paths"
					);
				}
			}

			ImGui::NextColumn();

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
					forget_asset_id_command<asset_id_type> cmd;
					cmd.forgotten_id = path_entry.id;
					cmd.built_description = 
						typesafe_sprintf("Stopped tracking %x", path_entry.get_full_path().to_display())
					;

					history.execute_new(std::move(cmd), in);
				}
			}

			ImGui::NextColumn();
		};

		auto do_section = [&](
			const auto& paths,
			const std::array<std::string, 3> labels,
			const std::optional<rgba> color = std::nullopt
		) {
			if (paths.empty()) {
				return;
			}

			if (color) {
				text_color(labels[0], *color);
			}
			else {
				text_disabled(labels[0]);
			}

			ImGui::NextColumn();
			text_disabled(labels[1]);
			ImGui::NextColumn();
			text_disabled(labels[2]);
			ImGui::NextColumn();
			ImGui::Separator();

			for (const auto& p : paths) {
				do_path(p);
			}

			ImGui::Separator();
		};

		if (browser_settings.show_orphaned) {
			do_section(
				missing_orphaned_paths,
				{ "Missing paths (orphaned)", "Last seen location", "Operations" },
				red
			);
		}

		do_section(
			missing_paths,
			{ "Missing paths", "Last seen location", "Used at" },
			red
		);

		if (browser_settings.show_orphaned) {
			do_section(
				orphaned_paths,
				{ "Orphaned images", "Location", "Operations" }
			);
		}

		do_section(
			used_paths,
			{ "Images", "Location", "Used at" }
		);
	}
	else {

	}
}
