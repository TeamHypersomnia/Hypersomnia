#pragma once
#include <filesystem>
#include <optional>
#include <string>
#include "augs/math/camera_cone.h"
#include "augs/filesystem/path.h"
#include "augs/string/typesafe_sprintf.h"

struct playtest_render_request {
	augs::path_type recording_folder;
	double from_secs = 0.0;
	double to_secs = 0.0;
	std::optional<camera_eye> camera;
	bool hide_hud = false;
};

inline augs::path_type get_playtest_render_script_path() {
	return std::filesystem::absolute("../cmake/render_playtest_replay.sh").lexically_normal();
}

inline augs::path_type get_running_game_executable_path() {
	std::error_code ignored;
	return std::filesystem::read_symlink("/proc/self/exe", ignored);
}

inline std::string make_playtest_render_command(const playtest_render_request& request) {
	auto command = typesafe_sprintf(
		"\"%x\" --game \"%x\" --recording \"%x\" --from %2f --to %2f",
		get_playtest_render_script_path().string(),
		get_running_game_executable_path().string(),
		std::filesystem::absolute(request.recording_folder).string(),
		request.from_secs,
		request.to_secs
	);

	if (request.camera.has_value()) {
		const auto& eye = *request.camera;
		command += typesafe_sprintf(" --camera %2f,%2f,%4f", eye.transform.pos.x, eye.transform.pos.y, eye.zoom);
	}

	if (request.hide_hud) {
		command += " --no-hud";
	}

	return command;
}
