#include "augs/string/string_templates.h"

#include "augs/templates/introspection_utils/field_name_tracker.h"
#include "augs/templates/introspection_utils/introspect_with_containers.h"

#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/path_tree_structs.h"

#include "application/setups/editor/gui/editor_assets_gui.h"
#include "application/setups/editor/editor_folder.h"
#include "application/intercosm.h"

struct gfx_browser_client {

};

template <class id_type>
struct asset_gui_path_entry : public browsed_path_entry_base {
	using base = browsed_path_entry_base;

	id_type id;

	asset_gui_path_entry() = default;
	asset_gui_path_entry(
		augs::path_type from,
	   	const id_type id
	) :
		id(id),
		base(from)
	{}
};

template <class S, class O, class F>
void find_object_in_object(
	const S& searched_object,
	const O& in_object,
	F location_callback
) {
	augs::field_name_tracker fields;

	auto callback = augs::recursive(
		[&searched_object, &location_callback, &fields](auto&& self, auto label, auto& field) {
			using T = std::decay_t<decltype(field)>;

			if constexpr(
				is_one_of_v<T, all_logical_assets, all_entity_flavours>
			) {
				/* This has a special logic */
			}
			else if constexpr(std::is_same_v<T, S>) {
				if (searched_object == field) {
					location_callback(fields.get_full_name(label));
				}
			}
			else if constexpr(!is_introspective_leaf_v<T>) {
				auto scope = fields.track(label);
				augs::introspect_with_containers(augs::recursive(self), field);
			}
		}
	);

	augs::introspect_with_containers(callback, in_object);

}

template <class F>
void find_locations_that_use(
	const assets::image_id id,
	const intercosm& inter,
	F location_callback
) {
	auto traverse = [&](const std::string& preffix, const auto& object) {
		find_object_in_object(id, object, [&](const auto& location) {
			location_callback(preffix + location);
		});
	};

	traverse("Common: ", inter.world.get_common_significant());
	//traverse("Particle effects: ", inter.viewables.particle_effects);
}

void editor_images_gui::perform(editor_command_input in) {
	using namespace augs::imgui;

	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	auto& work = *in.folder.work;
	auto& cosm = work.world;

	const auto& viewables = work.viewables;

	thread_local std::vector<asset_gui_path_entry<assets::image_id>> all_paths;
	all_paths.clear();

	viewables.image_loadables.for_each_object_and_id(
		[](const auto& object, const auto id) mutable {
			all_paths.emplace_back(object.source_image_path, id);
		}
	);

	sort_range(all_paths);

	browser_settings.do_tweakers();

	thread_local ImGuiTextFilter filter;
	filter.Draw();

	acquire_keyboard_once();

	auto files_view = scoped_child("Files view");

	auto& tree_settings = browser_settings.tree_settings;

	if (tree_settings.linear_view) {
		ImGui::Columns(3);

		tree_settings.do_name_location_columns();

		text_disabled("Used at");
		ImGui::NextColumn();

		ImGui::Separator();

		for (const auto& path_entry : all_paths) {
			const auto displayed_name = tree_settings.get_prettified(path_entry.get_filename());
			const auto displayed_dir = path_entry.get_displayed_directory();

			const auto id = path_entry.id;

			std::vector<std::string> locations;

			find_locations_that_use(id, work, [&](const std::string& location) {
				locations.push_back(location);
			});

			if (!filter.PassFilter(displayed_name.c_str()) && !filter.PassFilter(displayed_dir.c_str())) {
				continue;
			}

			if (!browser_settings.show_unused && locations.empty()) {
				continue;
			}

			{
				const auto node_label = typesafe_sprintf("%x###%x", displayed_name, path_entry.get_full_path());

				if (auto node = scoped_tree_node(node_label.c_str())) {

				}
			}

			ImGui::NextColumn();

			text_disabled(displayed_dir);

			ImGui::NextColumn();

			if (locations.empty()) {
				text_disabled("Nowhere");
			}
			else {
				if (auto node = scoped_tree_node(typesafe_sprintf("%x locations###%x", locations.size(), path_entry.id).c_str())) {
					for (const auto& l : locations) {
						text(l);
					}
				}
			}

			ImGui::NextColumn();
		}
	}
	else {

	}
}
