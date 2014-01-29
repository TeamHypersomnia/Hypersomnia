#include "texture_helper.h"

namespace topdown {
	texture_helper::texture_helper(std::wstring filename, augs::texture_baker::atlas& atl) {
		img.from_file(filename);
		tex.set(&img);
		atl.textures.push_back(&tex);
	}
}
