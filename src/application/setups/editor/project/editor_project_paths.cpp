#include "application/setups/editor/project/editor_project_paths.h"

editor_project_paths::editor_project_paths(const augs::path_type& target_folder) : project_folder(target_folder) {
	arena_name = target_folder.filename().string();

	auto in_folder = [&](const auto& rest) {
		return target_folder / rest;
	};

	auto in_cache = [&](const auto& rest) {
		return in_folder(".cache") / rest;
	};

	project_json = in_folder(arena_name + ".json");
	autosave_json = in_folder("autosave.json");
	editor_view = in_folder("editor_view.json");
	miniature = in_folder("miniature.png");
	signature = in_folder("signature");

	compressed_json = in_cache("compressed.lz4");
	resource_hashes = in_cache("resource_hashes.bin");
	fast_load_bin = in_cache("fast_load.bin");
}

bool editor_project_paths::is_project_specific_file(const augs::path_type& path) const {
	return 
		path == project_json
		|| path == autosave_json
		|| path == editor_view
		|| path == miniature
		|| path == signature
		|| path == compressed_json
		|| path == resource_hashes
		|| path == fast_load_bin
	;
}
