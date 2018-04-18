#include "augs/string/string_templates.h"

#include "augs/templates/introspection_utils/field_name_tracker.h"
#include "augs/templates/introspection_utils/introspect_with_containers.h"

#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/browse_path_tree.h"

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

template <class asset_id_type, class F>
void find_locations_that_use(
	const asset_id_type id,
	const cosmos& cosm,
	F&& location_callback
) {
	augs::field_name_tracker fields;

	auto by_introspection = [&](const auto& where) {
		auto callback = augs::recursive([&](auto&& self, auto label, auto& field) {
			using T = std::decay_t<decltype(field)>;

			if constexpr(
				is_one_of_v<T, all_logical_assets, all_entity_flavours>
			) {
				/* This has a special logic */
			}
			else if constexpr(std::is_same_v<T, asset_id_type>) {
				if (id == field) {
					location_callback(fields.get_full_name(label));
				}
			}
			else if constexpr(!is_introspective_leaf_v<T>) {
				auto scope = fields.track(label);
				augs::introspect_with_containers(augs::recursive(self), field);
			}
		});

		augs::introspect_with_containers(callback, where);
	};

	const auto& common = cosm.get_common_significant();

	by_introspection(common);
}

void editor_images_gui::perform(editor_command_input in) {
	using namespace augs::imgui;

	auto window = make_scoped_window();

	if (!window) {
		return;
	}

	acquire_keyboard_once();

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

	browse_path_tree(
		browser_settings.tree_settings,
		all_paths,
		[&](const auto& path_entry, const auto displayed_name) {
			const auto node_label = typesafe_sprintf("%x###%x", displayed_name, path_entry.get_full_path());

			if (auto node = scoped_tree_node(node_label.c_str())) {

			}
		},
		{},
		{ "Used at" },
		[&](const auto& path_entry) {
			const auto id = path_entry.id;

			std::vector<std::string> locations;

			find_locations_that_use(id, cosm, [&](const std::string& location) {
				locations.push_back(location);
			});

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
		}
	);
}
