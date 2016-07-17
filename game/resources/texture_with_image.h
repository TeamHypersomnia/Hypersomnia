#pragma once
#include "texture_baker/texture_baker.h"
#include "game/components/sprite_component.h"

namespace resources {
	class texture_with_image {
	public:
		augs::image img;
		augs::texture tex;
		std::vector<vec2i> polygonized;
		components::sprite gui_sprite_def;

		void set_from_image(augs::image img);
		void set_from_image_file(std::string filename);
		void polygonize_from_file(std::string filename);
	};
}