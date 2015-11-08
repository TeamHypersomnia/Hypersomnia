#pragma once
#include "texture_baker/texture_baker.h"
#include <string>
#include "math/vec2.h"

namespace helpers {
	struct texture_helper {
		augs::image img;
		augs::texture tex;

		vec2 get_size() const {
			return vec2(tex.get_size());
		}

		texture_helper(std::wstring filename, augs::atlas& atl);
	};
}
