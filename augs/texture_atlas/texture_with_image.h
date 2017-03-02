#pragma once
#include "augs/texture_atlas/texture_atlas_entry.h"
#include "augs/image/image.h"

#include "game/components/sprite_component.h"

namespace augs {
	class texture_with_image {
	public:
		augs::image img;
		augs::texture_atlas_entry tex;
		std::vector<vec2i> polygonized;
		components::sprite gui;

		void set_from_image(const augs::image& img);
		void set_from_image_file(std::string filename);
		void polygonize_from_file(const std::string& filename);
	};
}