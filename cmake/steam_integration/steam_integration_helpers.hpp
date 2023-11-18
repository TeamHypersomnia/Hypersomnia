#pragma once
#include "augs/image/image.h"
#include "augs/network/network_types.h"

inline std::string steam_get_launch_command_line_string() {
	char buffer[512];
	int num_bytes = steam_get_launch_command_line(buffer, sizeof(buffer));

	if (num_bytes > 0 && uint32_t(num_bytes) < sizeof(buffer)) {
		/* Exclude the terminating zero */
		return std::string(buffer, buffer + num_bytes - 1);
	}

	return std::string();
}

inline augs::image steam_get_avatar_image() {
	uint32_t w = 0;
	uint32_t h = 0;

	if (const auto rgba = steam_get_avatar(&w, &h)) {
		auto result = augs::image(rgba, vec2u(w, h));

		const auto max_s = static_cast<unsigned>(max_avatar_side_v);
		result.scale(vec2u::square(max_s));
		return result;
	}

	return {};
}
