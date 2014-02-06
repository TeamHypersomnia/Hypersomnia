#pragma once
#include "graphics/pixel.h"
#include "math/vec2d.h"
#include "../game/texture_helper.h"

using namespace augs;

namespace augs {
	namespace texture_baker {
		class texture;
	}
}

namespace resources {
	struct vertex {
		vec2<> pos;
		vec2<> texcoord;
		graphics::pixel_32 color;

		vertex() {}
		vertex(vec2<> pos) : pos(pos) {}
		vertex(vec2<> pos, vec2<> texcoord, graphics::pixel_32 color, texture_baker::texture* tex);

		void set_texcoord(vec2<>, helpers::texture_helper* tex);
	};

	struct vertex_triangle {
		vertex vertices[3];

		vertex& get_vert(int i) { return vertices[i]; }
	};

	typedef std::vector<vertex_triangle> buffer;
}
