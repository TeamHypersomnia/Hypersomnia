#pragma once
#include "graphics/pixel.h"
#include "math/vec2d.h"

using namespace augmentations;

namespace augmentations {
	namespace texture_baker {
		class texture;
	}
}

namespace resources {
	struct vertex {
		vec2<> position;
		vec2<> texcoord;
		graphics::pixel_32 color;

		vertex() {}
		vertex(vec2<> position, vec2<> texcoord, graphics::pixel_32 color, texture_baker::texture* tex);
	};

	struct vertex_triangle {
		vertex vertices[3];
	};

	typedef std::vector<vertex_triangle> buffer;
}
