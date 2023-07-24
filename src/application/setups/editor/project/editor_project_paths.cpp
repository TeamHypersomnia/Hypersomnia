#include "application/setups/editor/project/editor_project_paths.h"
#include "application/arena/arena_paths.h"
#include "application/setups/editor/editor_paths.h"

namespace sanitization {
	bool arena_name_safe(const std::string& untrusted_map_name);
}

std::string editor_project_paths::load_project_json() const {
	return augs::file_to_string_crlf_to_lf(project_json);
}

augs::path_type server_choose_arena_file_by(const std::string& name) {
	if (!sanitization::arena_name_safe(name)) {
		return {};
	}

	/*
		Prioritize EDITOR_PROJECTS_DIR over DOWNLOADED_ARENAS_DIR
		in case that the mapper downloads their own map hosted on another server.
		This will make spawning the online playtesting session work out of the box.
	*/

	const std::array<augs::path_type, 3> candidates = { 
		OFFICIAL_ARENAS_DIR,
		EDITOR_PROJECTS_DIR,
		DOWNLOADED_ARENAS_DIR
	};

	for (const auto& candidate_folder : candidates) {
		const auto arena_path = candidate_folder / name;

		if (augs::exists(arena_path)) {
			auto paths = editor_project_paths(arena_path);

			return paths.project_json;
		}
	}

	return {};
}

client_find_arena_result client_find_arena(
	const std::string& name,
	const augs::secure_hash_type& required_hash
) {
	if (!sanitization::arena_name_safe(name)) {
		client_find_arena_result result;
		result.invalid_arena_name = true;

		return result;
	}

	{
		const auto in_official = OFFICIAL_ARENAS_DIR / name;

		if (augs::exists(in_official)) {
			client_find_arena_result result;

			try {
				result.json_document = editor_project_paths(in_official).load_project_json();
				result.official_differs = required_hash != augs::secure_hash(result.json_document);

				if (!result.official_differs) {
					result.arena_folder_path = in_official;
				}
			}
			catch (...) {
				result.official_differs = true;
			}

			return result;
		}
	}

	for (const auto& candidate_folder : { EDITOR_PROJECTS_DIR, DOWNLOADED_ARENAS_DIR }) {
		const auto in_folder = candidate_folder / name;

		if (augs::exists(in_folder)) {
			try {
				client_find_arena_result result;
				result.arena_folder_path = in_folder;

				const auto folder_paths = editor_project_paths(in_folder);

				result.json_document = folder_paths.load_project_json();

				if (required_hash == augs::secure_hash(result.json_document)) {
					return result;
				}
			}
			catch (...) {

			}
		}
	}

	client_find_arena_result result;
	result.not_found_any = true;

	return result;
}

editor_project_paths::editor_project_paths(const augs::path_type& target_folder) : project_folder(target_folder) {
	arena_name = target_folder.filename().string();

	auto in_folder = [&](const auto& rest) {
		return target_folder / rest;
	};

	cache_folder = in_folder(".cache");

	auto in_cache = [&](const auto& rest) {
		return cache_folder / rest;
	};

	project_json = in_folder(arena_name + ".json");
	legacy_autosave_json = in_folder("autosave.json");
	last_saved_json = in_folder("last_saved.json");
	editor_view = in_folder("editor_view.json");
	miniature = in_folder("miniature.png");
	screenshot = in_folder("screenshot.png");
	signature = in_folder("signature");

	compressed_json = in_cache("compressed.lz4");
	resource_hashes = in_cache("resource_hashes.bin");
	fast_load_bin = in_cache("fast_load.bin");
}

bool editor_project_paths::is_project_specific_file(const augs::path_type& path) const {
	return 
		path == project_json
		|| path == legacy_autosave_json
		|| path == last_saved_json
		|| path == editor_view
		|| path == miniature
		|| path == screenshot
		|| path == signature
		|| path == compressed_json
		|| path == resource_hashes
		|| path == fast_load_bin
		|| path == cache_folder
	;
}
