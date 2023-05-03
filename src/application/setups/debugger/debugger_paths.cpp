#include "application/setups/debugger/debugger_paths.h"

debugger_paths::debugger_paths(
	const augs::path_type& target_folder,
	const augs::path_type& project_name
) : debugger_paths(target_folder, project_name.string()) {

}

debugger_paths::debugger_paths(
	const augs::path_type& target_folder,
	const std::string& project_name
) : arena(target_folder, project_name) {
	auto in_folder = [&](const auto ext) {
		return target_folder / (project_name + ext);
	};

	view_file = in_folder(".view");
	view_ids_file = in_folder(".view_ids");
	hist_file = in_folder(".hist");
	player_file = in_folder(".player");
	entropies_live_file = in_folder(".live");

	int_lua_file = in_folder(".int.lua");
	json_export_file = in_folder(".json");
	rulesets_lua_file = in_folder(".rulesets.lua");

	default_export_path = target_folder / (project_name + "-export");
	imported_folder_path = target_folder / (project_name + "-imported");

	version_info_file = in_folder(".version.txt");

	autosave_path = target_folder / "autosave";
}

