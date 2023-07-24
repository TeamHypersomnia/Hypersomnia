#pragma once
#include "augs/filesystem/path.h"
#include "application/arena/intercosm_paths.h"
#include "augs/misc/secure_hash.h"

struct editor_project_paths {
	std::string arena_name;
	augs::path_type project_folder;
	augs::path_type cache_folder;

	augs::path_type editor_view;

	augs::path_type project_json;

	augs::path_type last_saved_json;
	augs::path_type legacy_autosave_json;

	augs::path_type miniature;
	augs::path_type screenshot;

	augs::path_type signature;

	/* cache */
	augs::path_type compressed_json;
	augs::path_type resource_hashes;

	augs::path_type fast_load_bin;

	editor_project_paths(const augs::path_type& target_folder);

	bool is_project_specific_file(const augs::path_type&) const;

	std::string load_project_json() const;

	auto resolve(const augs::path_type& path) const {
		return project_folder / path;
	}
};

struct client_find_arena_result {
	bool official_differs = false;
	bool invalid_arena_name = false;
	bool not_found_any = false;

	std::optional<augs::path_type> arena_folder_path;
	std::string json_document;

	bool was_arena_found() const {
		return arena_folder_path.has_value();
	}
};

client_find_arena_result client_find_arena(
	const std::string& name,
	const augs::secure_hash_type& required_hash
);

augs::path_type server_choose_arena_file_by(const std::string& name);

