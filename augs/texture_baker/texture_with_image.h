#pragma once
#include "augs/texture_baker/texture.h"
#include "augs/texture_baker/image.h"

#include "game/components/sprite_component.h"

namespace augs {
	class texture_with_image {
	public:
		augs::image img;
		augs::texture tex;
		std::vector<vec2i> polygonized;
		components::sprite gui_sprite_def;

		void set_from_image(const augs::image& img);
		void set_from_image_file(std::string filename);
		void polygonize_from_file(const std::string& filename);
	};
}