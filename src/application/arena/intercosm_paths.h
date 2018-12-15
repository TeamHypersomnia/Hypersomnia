#pragma once
#include "augs/filesystem/path.h"

struct intercosm_paths {
	augs::path_type viewables_file;
	augs::path_type comm_file;
	augs::path_type solv_file;

	intercosm_paths(
		const augs::path_type& target_folder,
		const std::string& arena_name
	);
};
