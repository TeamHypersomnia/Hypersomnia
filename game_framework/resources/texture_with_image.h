#pragma once
#include "texture_baker/texture_baker.h"

namespace resources {
	class texture_with_image {
	public:
		augs::image img;
		augs::texture tex;
		std::vector<vec2i> polygonized;

		void set_from_image(augs::image img);
		void set_from_image_file(std::wstring filename);
		void polygonize_from_file(std::wstring filename);
	};
}