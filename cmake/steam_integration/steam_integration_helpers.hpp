#pragma once
#include "augs/image/image.h"

inline std::string steam_get_launch_command_line_string() {
	char buffer[512];
	int num_bytes = steam_get_launch_command_line(buffer, sizeof(buffer));

	if (num_bytes != 0) {
		return std::string(buffer, buffer + num_bytes);
	}

	return std::string();
}

inline augs::image steam_get_avatar() {
	uint32_t w = 0;
	uint32_t h = 0;

	if (const auto rgba = steam_get_avatar(&w, &h)) {
		auto result = augs::image(rgba, vec2u(w, h));
		delete [] rgba;

		return result;
	}

	return {};
}
