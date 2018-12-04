#pragma once
#include "augs/filesystem/path.h"

#define ARENAS_DIR "arenas"

struct arena_paths {
	augs::path_type int_file;
	augs::path_type rulesets_file;

	arena_paths(
		const augs::path_type& target_folder,
		const std::string& arena_name
	);

	arena_paths(const std::string& arena_name);
};
