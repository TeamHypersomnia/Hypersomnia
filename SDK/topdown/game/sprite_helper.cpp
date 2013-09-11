#include "sprite_helper.h"

sprite_helper::sprite_helper(std::wstring filename, augmentations::texture_baker::atlas& atl) {
	img.from_file(filename);
	tex.set(&img);
	atl.textures.push_back(&tex);
	set(&tex, graphics::pixel_32());
}
