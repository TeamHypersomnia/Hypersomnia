#include "texture_helper.h"

namespace helpers {
	texture_helper::texture_helper(std::wstring filename, augs::atlas& atl) {
		img.from_file(filename, 4);
		tex.set(&img);
		atl.textures.push_back(&tex);
	}
}
