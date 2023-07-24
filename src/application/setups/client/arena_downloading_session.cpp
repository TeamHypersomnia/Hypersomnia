#include "application/setups/client/arena_downloading_session.h"
#include "application/arena/arena_paths.h"
#include "augs/readwrite/byte_file.h"
#include "augs/filesystem/directory.h"
#include "application/setups/editor/project/editor_project_readwrite.h"
#include "application/setups/editor/project/editor_project_paths.h"
#include "augs/readwrite/json_readwrite_errors.h"
#include "augs/readwrite/memory_stream.h"
#include "augs/log.h"
#include "augs/readwrite/pointer_to_buffer.h"

arena_downloading_session::arena_downloading_session(
	const std::string& arena_name,
	const hash_or_timestamp& project_json_hash,
	arena_downloading_session::file_requester_type file_requester
) : 
	arena_name(arena_name),
	project_json_hash(project_json_hash),
	file_requester(file_requester)
{
	part_dir_path = DOWNLOADED_ARENAS_DIR / arena_name;
	part_dir_path += ".part";

	old_dir_path = DOWNLOADED_ARENAS_DIR / arena_name;
	old_dir_path += ".old";

	json_path_in_part_dir = part_dir_path / arena_name;
	json_path_in_part_dir += ".json";

	target_dir_path = DOWNLOADED_ARENAS_DIR / arena_name;

	start();
}

float arena_downloading_session::get_total_percent_complete(const float this_percent_complete) const {
	const auto downloaded_index = get_downloaded_file_index();
	const auto num_all = num_all_downloaded_files();

	if (num_all == 0) {
		/* Can't estimate if we don't even have the json file. */
		return 0.0f;
	}

	if (num_all == 1) {
		return this_percent_complete;
	}

	return (float(downloaded_index) + this_percent_complete) / num_all;
}

void arena_downloading_session::start() {
	arena_already_exists = augs::exists(target_dir_path);

	build_content_database_from_candidate_folders();

	augs::create_directories(part_dir_path);

	if (try_load_json_from_part_folder()) {
		if (const auto first_resource_to_download = next_to_download()) {
			request_file_download(*first_resource_to_download);
		}
		else {
			/* 
				Will likely never happen as it would mean the part folder was already complete.
				But you never know.
			*/

			finalize_arena_download();
		}
	}
	else {
		const auto json_url = arena_name + ".json";

		auto requested_project_json_hash = augs::secure_hash_type();

		if (const auto hash = std::get_if<augs::secure_hash_type>(&project_json_hash)) {
			requested_project_json_hash = *hash;
		}

		/* 
			Hash will be zero if we pass the timestamp instead of hash. 
			This will only happen in the context where the file requester cares only about the path (the https downloader).
		*/

		request_file_download({ requested_project_json_hash, json_url });
	}
}

void arena_downloading_session::finalize_arena_download() {
	if (const auto error = final_rearrange_directories()) {
		last_error = *error;
	}
}

bool arena_downloading_session::in_progress() const {
	if (last_error.has_value()) {
		return false;
	}

	return last_requested_file_hash.has_value();
} 

void arena_downloading_session::advance_with(
	const augs::cptr_memory_stream next_received_file
) {
	if (still_downloading_project_json()) {
		try {
			if (!handle_downloaded_project_json(next_received_file)) {
				return;
			}
		}
		catch (const augs::json_deserialization_error& err) {
			last_error = typesafe_sprintf("Server sent an invalid arena json file. Details:\n%x", err.what());
			return;
		}
		catch (...) {
			last_error = std::string("Server sent an invalid arena json file.");
			return;
		}
	}
	else {
		ensure(last_requested_file_hash.has_value());

		const auto requested = *last_requested_file_hash;
		const bool requested_hash_matches = requested == augs::secure_hash(next_received_file);

		if (requested_hash_matches) {
			create_files_matching_hash(requested, next_received_file);

			/* 
				Nullopt last_requested_file_hash will mark the end of download,
				if no more files will now be requested.
			*/

			last_requested_file_hash = std::nullopt;
		}
		else {
			last_error = typesafe_sprintf(
				"The server sent a file with an incorrect hash.\nExpected: %x\nActual: %x\n",
				last_requested_file_hash,
				augs::secure_hash(next_received_file)
			);
		}
	}

	if (const auto next_resource_to_download = next_to_download()) {
		request_file_download(*next_resource_to_download);
	}
	else {
		finalize_arena_download();
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

		register_content_in (target_dir_path, paths.project_json);
	}
}

void arena_downloading_session::request_file_download(const hash_and_url& entry) {
	const auto& hash = entry.first;
	const auto& url_in_provider = entry.second;

	last_requested_file_hash = hash;
	file_requester(hash, url_in_provider);
}

bool arena_downloading_session::try_load_json_from_part_folder() {
	try {
		const auto project_json = augs::file_to_string_crlf_to_lf(json_path_in_part_dir);

		if (project_json_matches(project_json_hash, project_json)) {
			determine_needed_resources_from(project_json);

			return true;
		}
	}
	catch (...) {

	}

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

void arena_downloading_session::determine_needed_resources_from(const std::string& project_json) {
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

bool arena_downloading_session::handle_downloaded_project_json(
	const augs::cptr_memory_stream bytes
) {
	last_requested_file_hash = std::nullopt;

	auto project_json = std::string(
		reinterpret_cast<const char*>(bytes.data()), 
		reinterpret_cast<const char*>(bytes.data() + bytes.size())
	);

	augs::crlf_to_lf(project_json);

	if (!project_json_matches(project_json_hash, project_json)) {
		last_error = typesafe_sprintf(
			"The server sent a project json file with an incorrect hash/timestamp.\nExpected: %x\nActual hash: %x\nActual timestamp: %x\n",
			project_json_hash,
			augs::secure_hash(project_json),
			editor_project_readwrite::read_only_project_timestamp(project_json)
		);

		return false;
	}

	determine_needed_resources_from(project_json);

	augs::save_as_text(json_path_in_part_dir, project_json);

	return true;
}

void arena_downloading_session::create_files_matching_hash(
	const augs::secure_hash_type& hash,
	const augs::cptr_memory_stream bytes
) {
	const auto& entry = output_files_by_hash.at(hash);

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
	if (still_downloading_project_json()) {
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

bool arena_downloading_session::project_json_matches(const hash_or_timestamp& ts, const std::string& project_json) {
	if (const auto required_hash = std::get_if<augs::secure_hash_type>(&ts)) {
		return augs::secure_hash(project_json) == *required_hash;
	}
	else if (const auto timestamp = std::get_if<version_timestamp_string>(&ts)) {
		try {
			return editor_project_readwrite::read_only_project_timestamp(project_json) == *timestamp;
		}
		catch (...) {
			return false;
		}
	}

	return false;
}

