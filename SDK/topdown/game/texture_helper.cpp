#include "texture_helper.h"

namespace topdown {
	texture_helper::texture_helper(std::wstring filename, augmentations::texture_baker::atlas& atl) {
		img.from_file(filename);
		set(&img);
		atl.textures.push_back(this);
	}
}
