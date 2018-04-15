#include "augs/string/string_templates.h"

#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/browse_path_tree.h"

#include "application/setups/editor/gui/editor_assets_gui.h"
#include "application/setups/editor/editor_folder.h"
#include "application/intercosm.h"

struct gfx_browser_client {

};

template <class id_type>
struct sorted_path_entry {
	std::string filename;
	std::string directory;
	id_type id;

	sorted_path_entry() = default;
	sorted_path_entry(
		augs::path_type from,
	   	const id_type id
	) :
		id(id),
		filename(from.filename()),
		directory(from.replace_filename(""))
	{}

	bool operator<(const sorted_path_entry& b) const {
		const auto& f1 = filename;
		const auto& f2 = b.filename;

		return std::tie(directory, f1) < std::tie(b.directory, f2);
	}

	const auto& get_filename() const {
		return filename;
	}

	const auto& get_directory() const {
		return directory;
	}

	auto get_full_path() const {
		return directory + filename;
	}
};

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

	thread_local std::vector<sorted_path_entry<assets::image_id>> all_paths;
	all_paths.clear();

	viewables.image_loadables.for_each_object_and_id(
		[](const auto& object, const auto id) mutable {
			all_paths.emplace_back(object.source_image_path, id);
		}
	);

	sort_range(all_paths);

	browse_path_tree(
		browser_settings,
		all_paths,
		[&](const auto& path_entry, const auto displayed_name) {
			const auto node_label = typesafe_sprintf("%x###%x", displayed_name, path_entry.get_full_path());

			if (auto node = scoped_tree_node(node_label.c_str())) {

			}
		}
	);
}
