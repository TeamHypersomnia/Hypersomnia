#pragma once
#include "texture_baker/texture_baker.h"
#include <string>
#include "math/vec2d.h"

namespace topdown {
	struct texture_helper {
		augmentations::texture_baker::image img;
		augmentations::texture_baker::texture tex;

		augmentations::vec2<> get_size() const {
			return augmentations::vec2<>(tex.get_size());
		}

		texture_helper(std::wstring filename, augmentations::texture_baker::atlas& atl);
	};
}
