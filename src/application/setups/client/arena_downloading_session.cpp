#include "application/setups/client/arena_downloading_session.h"
#include "application/arena/arena_paths.h"
#include "augs/readwrite/byte_file.h"
#include "augs/filesystem/directory.h"
#include "application/setups/editor/project/editor_project_readwrite.h"
#include "application/setups/editor/project/editor_project_paths.h"
#include "augs/readwrite/json_readwrite_errors.h"
#include "augs/log.h"

arena_downloading_session::arena_downloading_session(
	const std::string& arena_name,
	arena_downloading_session::file_requester_type file_requester
) : 
	arena_name(arena_name),
	file_requester(file_requester)
{
	part_dir_path = DOWNLOADED_ARENAS_DIR / arena_name;
	part_dir_path += ".part";

	old_dir_path = DOWNLOADED_ARENAS_DIR / arena_name;
	old_dir_path += ".old";

	json_path_in_part_dir = part_dir_path / arena_name;
	json_path_in_part_dir += ".json";

	target_dir_path = DOWNLOADED_ARENAS_DIR / arena_name;
}

bool arena_downloading_session::start(const augs::secure_hash_type& project_hash) {
	arena_already_exists = augs::exists(target_dir_path);

	build_content_database_from_candidate_folders();

	augs::create_directories(part_dir_path);

	if (try_load_json_from_part_folder(project_hash)) {
		if (const auto first_resource_to_download = next_to_download()) {
			request_file_download(*first_resource_to_download);

			return true;
		}
		else {
			/* 
				Will likely never happen as it would mean the part folder was already complete.
				But you never know.
			*/

			finalize_arena_download();
			return false;
		}
	}
	else {
		const auto json_url = arena_name + ".json";

		request_file_download({ project_hash, json_url });
		return true;
	}
}

void arena_downloading_session::finalize_arena_download() {
	if (const auto error = final_rearrange_directories()) {
		last_error = *error;
	}
}

bool arena_downloading_session::advance_with(
	const std::vector<std::byte>& next_received_file
) {
	const bool matches = requested_hash_matches(next_received_file);

	if (!matches) {
		last_error = typesafe_sprintf("The server sent a file with an incorrect hash.\nExpected: %x\nActual: %x\nDisconnecting.", augs::to_hex_format(last_requested_file_hash), augs::to_hex_format(augs::secure_hash(next_received_file)));

		return false;
	}

	const bool is_project_json_file = now_downloading_project_json();

	if (is_project_json_file) {
		try {
			/* 
				This could only fail due to read_only_external_resources throwing,
				which means the json file is ill-formed, unsafe or otherwise incorrect.
			*/

			handle_downloaded_project_json(next_received_file);
		}
		catch (const augs::json_deserialization_error& err) {
			last_error = typesafe_sprintf("Server sent an invalid arena json file. Disconnecting. Details:\n%x", err.what());
			return false;
		}
		catch (...) {
			last_error = std::string("Server sent an invalid arena json file. Disconnecting.");
			return false;
		}
	}
	else {
		create_files_from_downloaded(next_received_file);
	}

	if (const auto next_resource_to_download = next_to_download()) {
		request_file_download(*next_resource_to_download);

		return true;
	}
	else {
		finalize_arena_download();

		return false;
	}
}

void arena_downloading_session::build_content_database_from_candidate_folders() {
	auto register_content_in = [&](const auto& parent, const auto& json_path) {
		try {
			const auto externals = editor_project_readwrite::read_only_external_resources(
				parent,
				augs::file_to_string(json_path)
			);

			for (const auto& e : externals) {
				const auto hash = e.second;
				content_database[hash] = parent / e.first;
			}

			return true;
		}
		catch (...) {

		}

		return false;
	};

	if (arena_already_exists) {
		auto paths = editor_project_paths(target_dir_path);

		if (!register_content_in(target_dir_path, paths.autosave_json)) {
			register_content_in (target_dir_path, paths.project_json);
		}
	}
}

void arena_downloading_session::request_file_download(const hash_and_url& entry) {
	const auto& hash = entry.first;
	const auto& url_in_provider = entry.second;

	last_requested_file_hash = hash;
	file_requester(hash, url_in_provider);
}

bool arena_downloading_session::try_load_json_from_part_folder(
	const augs::secure_hash_type& required_hash
) {
	try {
		project_json = augs::file_to_string_crlf_to_lf(json_path_in_part_dir);

		if (augs::secure_hash(project_json) == required_hash) {
			determine_needed_resources();

			return true;
		}
	}
	catch (...) {

	}

	project_json.clear();
	return false;
}

std::optional<arena_downloading_session::hash_and_url> arena_downloading_session::next_to_download() {
	if (current_resource_idx == std::nullopt) {
		current_resource_idx = 0;
	}
	else {
		++(*current_resource_idx);
	}

	if (*current_resource_idx < all_needed_resources.size()) {
		try {
			const auto hash = all_needed_resources.at(*current_resource_idx);
			const auto entry = output_files_by_hash.at(hash);

			ensure(entry.output_files.size() > 0);

			return hash_and_url(hash, entry.output_files[0]);
		}
		catch (...) {
			last_error = std::string("Fatal failure in next_to_download.");
			ensure(false && "Fatal failure in next_to_download.");

			return std::nullopt;
		}
	}

	return std::nullopt;
}


bool arena_downloading_session::try_find_and_paste_file_locally(
	const augs::secure_hash_type& required_hash,
	const augs::path_type& path_in_project
) {
	/*
		Maybe it's already there.
		Can happen if the map was being downloaded before but interrupted halfway.
	*/

	const auto target_full_path = part_dir_path / path_in_project;

	try {
		if (required_hash == augs::secure_hash(augs::file_to_bytes(target_full_path))) {
			return true;
		}
	}
	catch (...) {
	
	}

	/*
		TODO: Properly read the original arena's json and determine by hash where to look for a candidate file.
		Only there get the path candidates and check if the hashes are indeed correct.
	*/

	try {
		if (const auto found_source_path = mapped_or_nullptr(content_database, required_hash)) {
			if (required_hash == augs::secure_hash(augs::file_to_bytes(*found_source_path))) {
				augs::create_directories_for(target_full_path);
				std::filesystem::copy(*found_source_path, target_full_path);

				return true;
			}
		}
	}
	catch (...) {

	}

	return false;
}

void arena_downloading_session::determine_needed_resources() {
	const auto& parent_folder_for_sanitization_checks = part_dir_path;

	const auto externals = editor_project_readwrite::read_only_external_resources(
		parent_folder_for_sanitization_checks,
		project_json
	);

	for (const auto& e : externals) {
		const auto hash = e.second;

		auto& entry = output_files_by_hash[hash];
		entry.output_files.push_back(e.first);

		if (entry.marked_for_download_already) {
			continue;
		}
		
		{
			const auto path_in_project = e.first;

			if (try_find_and_paste_file_locally(hash, path_in_project)) {
				continue;
			}
		}

		all_needed_resources.push_back(hash);
		entry.marked_for_download_already = true;
	}
}

bool arena_downloading_session::requested_hash_matches(
	const std::vector<std::byte>& bytes
) const {
	return augs::secure_hash(bytes) == last_requested_file_hash;
}

void arena_downloading_session::handle_downloaded_project_json(
	const std::vector<std::byte>& bytes
) {
	project_json.assign(
		reinterpret_cast<const char*>(&bytes[0]), 
		reinterpret_cast<const char*>(&bytes[0] + bytes.size())
	);

	determine_needed_resources();

	augs::save_as_text(json_path_in_part_dir, project_json);
}

void arena_downloading_session::create_files_from_downloaded(
	const std::vector<std::byte>& bytes
) {
	const auto& entry = output_files_by_hash.at(last_requested_file_hash);

	ensure(entry.marked_for_download_already);

	for (const auto& target_path : entry.output_files) {
		const auto full_path = part_dir_path / target_path;

		augs::create_directories_for(full_path);
		augs::bytes_to_file(bytes, full_path);
	}
}

std::optional<std::string> arena_downloading_session::final_rearrange_directories() {
	auto make_error = [&](const auto& err) {
		return typesafe_sprintf(
			"Failed to complete %x download:\n%x",
			arena_name,
			err.what()
		);
	};

	std::optional<std::string> result;

	auto try_remove = [&result, make_error](const auto& to_remove) {
		if (!augs::exists(to_remove)) {
			return true;
		}

		try {
			std::filesystem::remove_all(to_remove);
			return true;
		}
		catch (const std::filesystem::filesystem_error& err) {
			result = make_error(err);
		}
		catch (const std::exception& err) {
			result = make_error(err);
		}

		return false;
	};

	auto try_move = [&result, make_error](const auto& from, const auto& to) {
		try {
			std::filesystem::rename(from, to);
			return true;
		}
		catch (const std::filesystem::filesystem_error& err) {
			result = make_error(err) + typesafe_sprintf("\nCould not move:\n%x ->\n%x", from, to);
		}
		catch (const std::exception& err) {
			result = make_error(err);
		}

		return false;
	};

	if (try_remove(old_dir_path)) {
		if (augs::exists(target_dir_path)) {
			if (try_move(target_dir_path, old_dir_path)) {
				try_move(part_dir_path, target_dir_path);
			}
		}
		else {
			try_move(part_dir_path, target_dir_path);
		}
	}

	return result;
}

std::string arena_downloading_session::get_displayed_file_path() const {
	if (now_downloading_project_json()) {
		return json_path_in_part_dir.filename().string();
	}

	if (current_resource_idx) {
		if (*current_resource_idx < all_needed_resources.size()) {
			const auto currently_downloaded_hash = all_needed_resources[*current_resource_idx];

			if (const auto entry = mapped_or_nullptr(output_files_by_hash, currently_downloaded_hash)) {
				if (entry->output_files.size() > 0) {
					return entry->output_files[0].string();
				}
			}
		}
	}

	return "";
}

