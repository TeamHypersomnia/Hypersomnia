#pragma once
#include "graphics/pixel.h"
#include "math/vec2.h"

using namespace augs;

namespace augs {
	class texture;

	struct vertex {
		vec2 pos;
		vec2 texcoord;
		pixel_32 color;

		vertex() {}
		vertex(vec2 pos) : pos(pos) {}
		vertex(vec2 pos, vec2 texcoord, pixel_32 color, augs::texture& tex);

		void set_texcoord(vec2, augs::texture& tex);
	};

	struct vertex_triangle {
		vertex vertices[3];

		vertex& get_vert(int i) { return vertices[i]; }
	};

	typedef std::vector<vertex_triangle> vertex_triangle_buffer;
}
