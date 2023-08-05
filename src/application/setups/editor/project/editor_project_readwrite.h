#pragma once
#include "augs/filesystem/path.h"
#include "augs/misc/secure_hash.h"

struct editor_view;
struct editor_project;
struct editor_project_about;
struct editor_project_meta;

struct editor_resource_pools;
struct editor_official_resource_map;

namespace editor_project_readwrite {
	void write_project_json(const augs::path_type& json_path, const editor_project&);

	/*
		If strict is false, invalid entries will be ignored and it will load whatever is possible.
		Otherwise augs::json_deserialization_error will be thrown upon encountering any problem.
	*/

	struct reading_settings {
		bool strict = true;
		bool read_inactive_nodes = true;
	};

	editor_project read_project_json(
		const augs::path_type& parent_folder,
		const std::string& loaded_project_json,
		const editor_resource_pools& officials,
		const editor_official_resource_map& officials_map,
		const reading_settings settings = reading_settings(),
		augs::secure_hash_type* output_arena_hash = nullptr
	);

	editor_project read_project_json(
		const augs::path_type& json_path,
		const editor_resource_pools& officials,
		const editor_official_resource_map& officials_map,
		const reading_settings settings = reading_settings(),
		augs::secure_hash_type* output_arena_hash = nullptr
	);

	void write_project_json(
		const augs::path_type& json_path,
		const editor_project& project,
		const editor_resource_pools& officials,
		const editor_official_resource_map& officials_map
	);

	editor_project_about read_only_project_about(const augs::path_type& json_path);
	editor_project_meta read_only_project_meta(const augs::path_type& json_path);
	version_timestamp_string read_only_project_timestamp(const std::string& project_json);

	using external_resource_database = std::vector<std::pair<augs::path_type, augs::secure_hash_type>>;

	external_resource_database read_only_external_resources(
		const augs::path_type& parent_folder_for_sanitization_checks,
		const std::string& loaded_project_json
	);

	void write_editor_view(const augs::path_type& json_path, const editor_view&);
	editor_view read_editor_view(const augs::path_type& json_path);
}

