#include "application/setups/editor/editor_paths.h"

editor_paths::editor_paths(
	const augs::path_type& target_folder,
	const std::string& project_name
) {
	auto in_folder = [&](const auto ext) {
		return target_folder / (project_name + ext);
	};

	int_file = in_folder(".int");
	view_file = in_folder(".view");
	view_ids_file = in_folder(".view_ids");
	hist_file = in_folder(".hist");
	modes_file = in_folder(".modes");
	player_file = in_folder(".player");

	autosave_path = target_folder / "autosave";
}

