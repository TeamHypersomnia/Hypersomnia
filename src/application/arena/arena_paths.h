#pragma once
#include "augs/filesystem/path.h"
#include "application/arena/intercosm_paths.h"

#define USER_DOWNLOADS_DIR 		(augs::path_type(USER_FILES_DIR) 		/ "downloads")
#define OFFICIAL_ARENAS_DIR  	(augs::path_type(OFFICIAL_CONTENT_DIR) 	/ "arenas")
#define DOWNLOADED_ARENAS_DIR 	(USER_DOWNLOADS_DIR      				/ "arenas")

struct arena_paths {
	intercosm_paths int_paths;
	augs::path_type folder_path;

	augs::path_type rulesets_file_path;
	augs::path_type miniature_file_path;

	arena_paths(
		const augs::path_type& target_folder
	);

	arena_paths(
		const augs::path_type& target_folder,
		const std::string& arena_name
	);

	arena_paths(const std::string& arena_name);
};
