#pragma once
#include "texture_baker/texture_baker.h"
#include <string>

namespace topdown {
	struct texture_helper : augmentations::texture_baker::texture {
		augmentations::texture_baker::image img;

		texture_helper(std::wstring filename, augmentations::texture_baker::atlas& atl);
	};
}
