#pragma once
#include <algorithm>
#include <filesystem>
#include <optional>
#include <set>
#include <string>
#include <vector>
#include "augs/filesystem/directory.h"
#include "augs/filesystem/file.h"
#include "augs/misc/date_time.h"
#include "augs/misc/secure_hash.h"
#include "augs/readwrite/json_readwrite.h"
#include "augs/readwrite/lz4_file.h"
#include "augs/readwrite/to_bytes.h"
#include "augs/log.h"
#include "application/setups/editor/playtest_recording/editor_playtest_recording.h"

constexpr std::size_t max_kept_unnamed_recordings_v = 1000;
constexpr std::size_t map_version_hex_length_v = 16;

struct found_recorded_playtests {
	std::vector<editor_recorded_playtest_entry> entries;
	bool some_unreadable = false;
};

inline found_recorded_playtests find_all_recorded_playtests(const augs::path_type& recordings_folder) {
	auto found = found_recorded_playtests();

	if (!augs::exists(recordings_folder)) {
		return found;
	}

	auto skip_folder = [](auto&&...) { return callback_result::CONTINUE; };

	try {
		augs::for_each_in_directory(
			recordings_folder,
			skip_folder,
			[&](const auto& file) {
				if (!::is_recording_meta_json(file)) {
					return callback_result::CONTINUE;
				}

				const auto recording_paths = editor_playtest_recording_paths(file);

				try {
					found.entries.push_back({ recording_paths, augs::from_json_file<editor_playtest_recording_meta>(file) });
				}
				catch (const std::exception& err) {
					found.some_unreadable = true;
					LOG("Skipping the recorded playtest %x: %x", file, err.what());
				}

				return callback_result::CONTINUE;
			}
		);
	}
	catch (const std::exception& err) {
		found.some_unreadable = true;
		LOG("Could not list the recorded playtests in %x: %x", recordings_folder, err.what());
	}

	std::sort(found.entries.begin(), found.entries.end(), [](const auto& a, const auto& b) {
		return a.meta.recorded_at_unix > b.meta.recorded_at_unix;
	});

	return found;
}

inline std::vector<editor_recorded_playtest_entry> find_recorded_playtests(const augs::path_type& recordings_folder) {
	return ::find_all_recorded_playtests(recordings_folder).entries;
}

inline bool lies_in_playtest_recordings_folder(const augs::path_type& recording_json) {
	const auto recordings_folder = recording_json.lexically_normal().parent_path();
	const auto project_folder = ::project_folder_of_recording(recording_json);

	return recordings_folder == ::get_playtest_recordings_folder(project_folder);
}

inline void remove_recorded_playtest(const editor_playtest_recording_paths& recording_paths) {
	if (::lies_in_playtest_recordings_folder(recording_paths.meta_json)) {
		augs::remove_file(recording_paths.meta_json);
		augs::remove_file(recording_paths.inputs_lz4);
	}
}

inline void remove_unreferenced_map_versions(const augs::path_type& project_folder) {
	const auto found = ::find_all_recorded_playtests(::get_playtest_recordings_folder(project_folder));

	if (found.some_unreadable) {
		return;
	}

	auto referenced = std::set<augs::path_type>();

	for (const auto& entry : found.entries) {
		referenced.emplace(::get_playtest_map_version_path(project_folder, entry.meta.map_version).filename());
	}

	const auto map_versions_folder = ::get_playtest_map_versions_folder(project_folder);

	if (!augs::exists(map_versions_folder)) {
		return;
	}

	auto unreferenced = std::vector<augs::path_type>();

	try {
		augs::for_each_in_directory(
			map_versions_folder,
			[](auto&&...) { return callback_result::CONTINUE; },
			[&](const auto& file) {
				if (referenced.find(file.filename()) == referenced.end()) {
					unreferenced.push_back(file);
				}

				return callback_result::CONTINUE;
			}
		);

		for (const auto& file : unreferenced) {
			augs::remove_file(file);
		}
	}
	catch (const std::exception& err) {
		LOG("Could not remove the unused map versions in %x: %x", map_versions_folder, err.what());
	}
}

inline std::size_t count_recordings(const augs::path_type& recordings_folder) {
	auto num_recordings = std::size_t(0);

	augs::for_each_in_directory(
		recordings_folder,
		[](auto&&...) { return callback_result::CONTINUE; },
		[&](const auto& file) {
			if (::is_recording_meta_json(file)) {
				++num_recordings;
			}

			return callback_result::CONTINUE;
		}
	);

	return num_recordings;
}

inline bool remove_oldest_unnamed_recordings(const augs::path_type& recordings_folder) {
	if (::count_recordings(recordings_folder) <= max_kept_unnamed_recordings_v) {
		return false;
	}

	auto unnamed_kept = std::size_t(0);
	auto removed_any = false;

	for (const auto& entry : ::find_recorded_playtests(recordings_folder)) {
		if (!entry.meta.name.empty()) {
			continue;
		}

		if (++unnamed_kept > max_kept_unnamed_recordings_v) {
			::remove_recorded_playtest(entry.paths);
			removed_any = true;
		}
	}

	return removed_any;
}

inline augs::path_type first_free_recording_json(const augs::path_type& recordings_folder, const augs::date_time when) {
	const auto base_name = when.get_readable_for_file_long();
	auto candidate = recordings_folder / (base_name + ".json");

	for (int suffix = 2; augs::exists(candidate); ++suffix) {
		candidate = recordings_folder / typesafe_sprintf("%x_%x.json", base_name, suffix);
	}

	return candidate;
}

inline std::string calc_map_version(const std::string& map_json) {
	return std::string(augs::to_hex_format(augs::secure_hash(map_json))).substr(0, map_version_hex_length_v);
}

inline std::string save_map_version(const augs::path_type& project_folder, const std::string& map_json) {
	const auto map_version = ::calc_map_version(map_json);
	const auto path = ::get_playtest_map_version_path(project_folder, map_version);

	augs::create_directories(path.parent_path());
	augs::bytes_to_lz4_file_atomic(augs::string_to_bytes(map_json), path);

	return map_version;
}

inline std::string load_map_version(const editor_playtest_recording_paths& recording_paths, const std::string& map_version) {
	const auto bytes = augs::lz4_file_to_bytes(recording_paths.get_map_version_path(map_version));
	return std::string(reinterpret_cast<const char*>(bytes.data()), bytes.size());
}

inline void save_recording_inputs(const editor_playtest_recording_inputs& inputs, const augs::path_type& path) {
	auto bytes = std::vector<std::byte>();
	augs::to_bytes(bytes, inputs);
	augs::bytes_to_lz4_file_atomic(bytes, path);
}

inline editor_playtest_recording_inputs load_recording_inputs(const augs::path_type& path) {
	auto inputs = editor_playtest_recording_inputs();
	augs::from_bytes(augs::lz4_file_to_bytes(path), inputs);
	return inputs;
}
