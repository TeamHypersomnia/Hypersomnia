#pragma once
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
	augs::path_type recording_folder;
	double secs = 0.0;
	uint32_t step = 0;
	uint32_t num_steps = 0;
	std::optional<camera_eye> camera;
};

inline std::string make_playtest_moment_link(const playtest_moment& moment) {
	const auto recording = std::filesystem::absolute(moment.recording_folder).string();

	auto camera_field = std::string();
	auto camera_arg = std::string();

	if (moment.camera.has_value()) {
		const auto& eye = *moment.camera;
		camera_field = typesafe_sprintf(" camera=%2f,%2f,%4f", eye.transform.pos.x, eye.transform.pos.y, eye.zoom);
		camera_arg = typesafe_sprintf(" --replay-camera %2f,%2f,%4f", eye.transform.pos.x, eye.transform.pos.y, eye.zoom);
	}

	return typesafe_sprintf(
		"%x map=%x recording=\"%x\" at=%3fs step=%x/%x%x | --replay-playtest \"%x\" --replay-from %3f%x",
		playtest_moment_link_prefix_v,
		moment.map_name,
		recording,
		moment.secs,
		moment.step,
		moment.num_steps,
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

inline std::optional<playtest_replay_capture_settings> parse_playtest_moment_link(const std::string& link) {
	if (!begins_with(link, playtest_moment_link_prefix_v)) {
		return std::nullopt;
	}

	const auto recording = ::find_playtest_moment_link_field(link, " recording=\"", '"');
	const auto at = ::find_playtest_moment_link_field(link, " at=", 's');

	if (!recording.has_value() || !at.has_value()) {
		return std::nullopt;
	}

	auto request = playtest_replay_capture_settings();
	request.recording_folder = *recording;
	request.from_secs = std::max(0.0, std::atof(at->c_str()));
	request.start_paused = true;

	if (const auto camera = ::find_playtest_moment_link_field(link, " camera=", ' ')) {
		request.camera = ::parse_playtest_replay_camera(*camera);
	}

	if (!augs::exists(editor_playtest_recording_paths(request.recording_folder).meta_json)) {
		return std::nullopt;
	}

	return request;
}

class playtest_moment_link_watcher {
	uint32_t activations_seen = 0;
	bool checked_once = false;
	std::string last_taken;

public:
	bool worth_reading(const uint32_t times_activated) {
		const bool activated = times_activated != std::exchange(activations_seen, times_activated);
		const bool first_check = !std::exchange(checked_once, true);

		return activated || first_check;
	}

	std::optional<playtest_replay_capture_settings> take(const std::string& clipboard) {
		if (clipboard == last_taken) {
			return std::nullopt;
		}

		auto request = ::parse_playtest_moment_link(clipboard);

		if (request.has_value()) {
			last_taken = clipboard;
		}

		return request;
	}
};
