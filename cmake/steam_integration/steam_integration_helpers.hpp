#pragma once
#include "augs/image/image.h"

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
