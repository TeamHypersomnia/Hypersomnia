#include "application/setups/editor/project/editor_project_paths.h"

editor_project_paths::editor_project_paths(const augs::path_type& target_folder) : folder_path(target_folder) {
	arena_name = target_folder.filename().string();

	auto in_folder = [&](const auto& rest) {
		return target_folder / rest;
	};

	auto in_cache = [&](const auto& rest) {
		return in_folder(".cache") / rest;
	};

	project_json = in_folder(arena_name + ".json");
	miniature = in_folder(arena_name + ".miniature.png");
	signature = in_folder(arena_name + ".signature");

	compressed_json = in_cache(arena_name + ".lz4");
	resource_hashes = in_cache("resource_hashes.bin");
	fast_load_bin = in_cache("fast_load.bin");
}
