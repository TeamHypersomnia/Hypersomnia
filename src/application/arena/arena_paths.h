#pragma once
#include "augs/filesystem/path.h"
#include "application/arena/intercosm_paths.h"

#define OFFICIAL_ARENAS_DIR "arenas"
#define COMMUNITY_ARENAS_DIR "community_arenas"

struct arena_paths {
	intercosm_paths int_paths;
	augs::path_type folder_path;

	augs::path_type rulesets_file_path;
	augs::path_type miniature_file_path;

	arena_paths(
		const augs::path_type& target_folder,
		const std::string& arena_name
	);

	arena_paths(const std::string& arena_name);
};
