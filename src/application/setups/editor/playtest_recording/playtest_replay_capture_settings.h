#pragma once
#include <algorithm>
#include <cstdint>
#include <optional>
#include <sstream>
#include <string>
#include "augs/math/camera_cone.h"
#include "augs/filesystem/path.h"
#include "augs/misc/timing/delta.h"

constexpr unsigned playtest_replay_capture_frames_per_second_v = 60;
constexpr unsigned playtest_replay_capture_audio_frequency_v = 48000;

inline augs::delta playtest_replay_capture_frame_delta() {
	return augs::delta::steps_per_second(playtest_replay_capture_frames_per_second_v);
}

struct playtest_replay_capture_settings {
	augs::path_type recording_folder;
	double from_secs = 0.0;
	std::optional<double> to_secs;
	augs::path_type frames_path;
	augs::path_type audio_path;
	std::optional<camera_eye> camera;

	bool requested() const {
		return !recording_folder.empty();
	}

	bool records_frames() const {
		return !frames_path.empty();
	}

	bool records_audio() const {
		return !audio_path.empty();
	}
};

inline augs::path_type project_folder_of_recording(const augs::path_type& recording_folder) {
	auto folder = recording_folder.lexically_normal();

	if (folder.filename().empty()) {
		folder = folder.parent_path();
	}

	return folder.parent_path().parent_path();
}

inline std::optional<camera_eye> parse_playtest_replay_camera(std::string x_y_zoom) {
	std::replace(x_y_zoom.begin(), x_y_zoom.end(), ',', ' ');

	auto in = std::istringstream(x_y_zoom);
	auto eye = camera_eye();

	if (in >> eye.transform.pos.x >> eye.transform.pos.y >> eye.zoom) {
		return eye;
	}

	return std::nullopt;
}
