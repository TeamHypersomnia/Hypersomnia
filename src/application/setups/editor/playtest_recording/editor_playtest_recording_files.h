#pragma once
#include <algorithm>
#include <filesystem>
#include <vector>
#include "augs/filesystem/directory.h"
#include "augs/misc/date_time.h"
#include "augs/readwrite/json_readwrite.h"
#include "augs/log.h"
#include "application/setups/editor/playtest_recording/editor_playtest_recording.h"

constexpr std::size_t max_kept_unnamed_recordings_v = 50;

inline std::vector<editor_recorded_playtest_entry> find_recorded_playtests(const augs::path_type& recordings_folder) {
	auto found = std::vector<editor_recorded_playtest_entry>();

	if (!augs::exists(recordings_folder)) {
		return found;
	}

	auto skip_file = [](auto&&...) { return callback_result::CONTINUE; };

	try {
		augs::for_each_in_directory(
			recordings_folder,
			[&](const auto& folder) {
				const auto recording_paths = editor_playtest_recording_paths(folder);

				if (augs::exists(recording_paths.meta_json) && augs::exists(recording_paths.inputs_bin)) {
					try {
						found.push_back({ recording_paths, augs::from_json_file<editor_playtest_recording_meta>(recording_paths.meta_json) });
					}
					catch (const std::exception& err) {
						LOG("Skipping the recorded playtest %x: %x", folder, err.what());
					}
				}

				return callback_result::CONTINUE;
			},
			skip_file
		);
	}
	catch (const std::exception& err) {
		LOG("Could not list the recorded playtests in %x: %x", recordings_folder, err.what());
	}

	std::sort(found.begin(), found.end(), [](const auto& a, const auto& b) {
		return a.meta.recorded_at_unix > b.meta.recorded_at_unix;
	});

	return found;
}

inline void remove_recorded_playtest_folder(const editor_playtest_recording_paths& recording_paths) {
	const auto parent_named_like_recordings = recording_paths.folder.parent_path().filename() == ::get_recorded_playtests_folder("").filename();

	if (parent_named_like_recordings) {
		std::filesystem::remove_all(recording_paths.folder);
	}
}

inline void remove_unfinished_recordings(const augs::path_type& recordings_folder) {
	auto skip_file = [](auto&&...) { return callback_result::CONTINUE; };

	augs::for_each_in_directory(
		recordings_folder,
		[&](const auto& folder) {
			const auto recording_paths = editor_playtest_recording_paths(folder);

			if (!augs::exists(recording_paths.meta_json)) {
				::remove_recorded_playtest_folder(recording_paths);
			}

			return callback_result::CONTINUE;
		},
		skip_file
	);
}

inline void remove_oldest_unnamed_recordings(const augs::path_type& recordings_folder) {
	::remove_unfinished_recordings(recordings_folder);

	std::size_t unnamed_kept = 0;

	for (const auto& entry : ::find_recorded_playtests(recordings_folder)) {
		if (!entry.meta.name.empty()) {
			continue;
		}

		if (++unnamed_kept > max_kept_unnamed_recordings_v) {
			::remove_recorded_playtest_folder(entry.paths);
		}
	}
}

inline augs::path_type first_free_recording_folder(const augs::path_type& recordings_folder, const augs::date_time when) {
	const auto base_name = when.get_readable_for_file_long();
	auto candidate = recordings_folder / base_name;

	for (int suffix = 2; augs::exists(candidate); ++suffix) {
		candidate = recordings_folder / typesafe_sprintf("%x_%x", base_name, suffix);
	}

	return candidate;
}
