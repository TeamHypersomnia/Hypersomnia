#pragma once
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include "augs/math/camera_cone.h"
#include "augs/filesystem/path.h"
#include "augs/filesystem/file.h"
#include "augs/string/typesafe_sprintf.h"
#include "augs/string/string_templates.h"
#include "application/setups/editor/playtest_recording/playtest_replay_capture_settings.h"
#include "application/setups/editor/playtest_recording/editor_playtest_recording.h"

constexpr auto playtest_moment_link_prefix_v = "[hypersomnia replay moment]";

struct playtest_moment {
	std::string map_name;
	augs::path_type recording_json;
	double secs = 0.0;
	std::optional<camera_eye> camera;
};

inline std::string make_playtest_moment_link(const playtest_moment& moment) {
	const auto recording = augs::string_windows_friendly(std::filesystem::absolute(moment.recording_json));

	auto camera_field = std::string();
	auto camera_arg = std::string();

	if (moment.camera.has_value()) {
		const auto& eye = *moment.camera;
		camera_field = typesafe_sprintf(" camera=%2f,%2f,%4f", eye.transform.pos.x, eye.transform.pos.y, eye.zoom);
		camera_arg = typesafe_sprintf(" --replay-camera %2f,%2f,%4f", eye.transform.pos.x, eye.transform.pos.y, eye.zoom);
	}

	return typesafe_sprintf(
		"%x map=%x recording=\"%x\" at=%3fs%x | --replay-playtest \"%x\" --replay-from %3f%x",
		playtest_moment_link_prefix_v,
		moment.map_name,
		recording,
		moment.secs,
		camera_field,
		recording,
		moment.secs,
		camera_arg
	);
}

inline std::optional<std::string> find_playtest_moment_link_field(const std::string& link, const std::string& key, const char terminator) {
	const auto start = link.find(key);

	if (start == std::string::npos) {
		return std::nullopt;
	}

	const auto value_start = start + key.size();
	const auto value_end = link.find(terminator, value_start);

	return link.substr(value_start, value_end == std::string::npos ? std::string::npos : value_end - value_start);
}

inline bool is_path_separator(const char character) {
	return character == '/' || character == '\\';
}

inline bool starts_on_a_local_disk(const std::string& path) {
#if PLATFORM_WINDOWS
	return
		path.size() >= 3 &&
		std::isalpha(static_cast<unsigned char>(path[0])) &&
		path[1] == ':' &&
		::is_path_separator(path[2])
	;
#else
	const bool starts_with_two_separators =
		path.size() >= 2 &&
		::is_path_separator(path[0]) &&
		::is_path_separator(path[1])
	;

	return !starts_with_two_separators;
#endif
}

inline std::optional<playtest_replay_capture_settings> parse_playtest_moment_link(const std::string& link) {
	if (!begins_with(link, playtest_moment_link_prefix_v)) {
		return std::nullopt;
	}

	const auto recording = ::find_playtest_moment_link_field(link, " recording=\"", '"');
	const auto at = ::find_playtest_moment_link_field(link, " at=", 's');

	if (!recording.has_value() || !at.has_value()) {
		return std::nullopt;
	}

	if (!::starts_on_a_local_disk(*recording)) {
		return std::nullopt;
	}

	auto request = playtest_replay_capture_settings();
	request.recording_json = augs::path_windows_friendly(*recording);
	request.from_secs = std::max(0.0, std::atof(at->c_str()));
	request.start_paused = true;

	if (const auto camera = ::find_playtest_moment_link_field(link, " camera=", ' ')) {
		request.camera = ::parse_playtest_replay_camera(*camera);
	}

	const bool looks_like_a_recording =
		::is_recording_meta_json(request.recording_json) &&
		::is_in_recordings_folder_of_its_project(request.recording_json)
	;

	if (!looks_like_a_recording || !augs::exists(request.recording_json)) {
		return std::nullopt;
	}

	const auto project_folder = ::project_folder_of_recording(request.recording_json);
	auto project_json = project_folder / project_folder.filename();
	project_json += ".json";

	if (!augs::exists(project_json)) {
		return std::nullopt;
	}

	return request;
}

class playtest_moment_link_watcher {
	uint32_t activations_seen = 0;
	std::string last_taken;

public:
	bool window_activated(const uint32_t times_activated) {
		return times_activated != std::exchange(activations_seen, times_activated);
	}

	std::optional<playtest_replay_capture_settings> take(const std::string& clipboard) {
		const auto link = ::cut_trailing(std::string(clipboard), " \t\r\n");

		if (link == last_taken) {
			return std::nullopt;
		}

		auto request = ::parse_playtest_moment_link(link);

		if (request.has_value()) {
			last_taken = link;
		}

		return request;
	}
};
