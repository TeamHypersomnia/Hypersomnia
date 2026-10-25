#pragma once
#include <cstddef>
#include <vector>
#include "augs/image/image.h"
#include "augs/network/network_types.h"

/*
	A player's avatar as the server passes it on - to the site's match reports and a tournament's page:
	decoded and encoded again as PNG, so only a picture leaves, whatever was sent. Empty if it isn't one,
	or bigger than avatars may be.
*/

inline std::vector<std::byte> reencoded_avatar_png(const std::vector<std::byte>& bytes) {
	if (bytes.empty()) {
		return {};
	}

	try {
		auto image = augs::image();
		image.from_bytes_stbi(bytes, "avatar");

		if (image.get_size().x <= max_avatar_side_v && image.get_size().y <= max_avatar_side_v) {
			return image.to_png_bytes();
		}
	}
	catch (...) {

	}

	return {};
}
