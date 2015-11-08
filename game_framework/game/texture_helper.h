#pragma once
#include "texture_baker/texture_baker.h"
#include <string>
#include "math/vec2.h"

namespace helpers {
	struct texture_helper {
		augs::texture_baker::image img;
		augs::texture_baker::texture tex;

		augs::vec2<> get_size() const {
			return augs::vec2<>(tex.get_size());
		}

		texture_helper(std::wstring filename, augs::texture_baker::atlas& atl);
	};
}
