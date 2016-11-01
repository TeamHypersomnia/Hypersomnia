#pragma once
#include <vector>
#include "3rdparty/rectpack2D/src/pack.h"

#include "image.h"
#include "texture.h"

namespace augs {
	class texture_with_image;

	class atlas {
		static unsigned current;
		unsigned id = 0;
		bool mipmaps = false, lin = false, rep = true, built = false;
		std::vector<bin> bins;

	public:
		~atlas();

		std::vector<texture_with_image*> textures;

		texture atlas_texture;
		image img;

		bool pack(), // max texture size 
			pack(int max_size);

		void create_image(int atlas_channels, bool destroy_images);
		void build(bool mipmaps = false, bool linear = false, image* raw_texture = 0), bind(), _bind(), nearest(), linear(), clamp(), repeat();

		void default_build();

		bool is_mipmapped() const;
		void destroy();
	};
}