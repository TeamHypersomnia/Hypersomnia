#pragma once
#include <functional>
#include <vector>
#include <string>
#include <optional>
#include "augs/filesystem/path.h"
#include "augs/misc/secure_hash.h"

struct arena_downloading_session {
	using file_requester_type = std::function<void(const augs::secure_hash_type&, const augs::path_type&)>;

	struct file_hash_info {
		bool marked_for_download_already = false;
		std::vector<augs::path_type> output_files;
	};

	std::string arena_name;

	augs::path_type json_path_in_part_dir;

	augs::path_type old_dir_path;
	augs::path_type part_dir_path;
	augs::path_type target_dir_path;
	bool arena_already_exists = false;

	std::string project_json;

	augs::secure_hash_type last_requested_file_hash;
	std::vector<augs::secure_hash_type> all_needed_resources;
	std::optional<std::size_t> current_resource_idx;

	std::unordered_map<augs::secure_hash_type, file_hash_info> output_files_by_hash;
	std::unordered_map<augs::secure_hash_type, augs::path_type> content_database;

	using hash_and_url = std::pair<augs::secure_hash_type, augs::path_type>;

	arena_downloading_session(
		const std::string& arena_name,
		file_requester_type file_requester
	);

	bool start(const augs::secure_hash_type&);

	bool advance_with(
		const std::vector<std::byte>& next_received_file
	);

	bool has_errors() const {
		return last_error.has_value();
	}

	auto get_error() const {
		return last_error.has_value() ? *last_error : std::string();
	}

	std::size_t get_downloaded_file_index() const {
		if (current_resource_idx) {
			return *current_resource_idx;
		}

		return 0;
	}

	std::size_t num_all_downloaded_files() const {
		return all_needed_resources.size();
	}

	std::string get_displayed_file_path() const;

	bool now_downloading_project_json() const {
		return current_resource_idx == std::nullopt;
	}

	bool now_downloading_external_resources() const {
		return current_resource_idx.has_value();
	}

private:
	file_requester_type file_requester;

	std::optional<std::string> last_error;

	void finalize_arena_download();

	void request_file_download(const hash_and_url&);

	void build_content_database_from_candidate_folders();
	bool try_load_json_from_part_folder(const augs::secure_hash_type&);

	std::optional<hash_and_url> next_to_download();

	void handle_downloaded_project_json(const std::vector<std::byte>&);
	void create_files_from_downloaded(const std::vector<std::byte>&);
	
	void determine_needed_resources();

	bool try_find_and_paste_file_locally(
		const augs::secure_hash_type& hash,
		const augs::path_type& path_in_project
	);

	bool requested_hash_matches(const std::vector<std::byte>&) const;

	std::optional<std::string> final_rearrange_directories();
};
