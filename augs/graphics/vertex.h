#pragma once
#include "graphics/pixel.h"
#include "math/vec2.h"

using namespace augs;

namespace augs {
	class texture;

	struct vertex {
		vec2 pos;
		vec2 texcoord;
		rgba color;

		vertex() {}
		vertex(vec2 pos) : pos(pos) {}
		vertex(vec2 pos, vec2 texcoord, rgba color, augs::texture& tex);

		void set_texcoord(vec2, augs::texture& tex);
	};

	struct vertex_triangle {
		vertex vertices[3];

		vertex& get_vert(int i) { return vertices[i]; }
	};

	struct vertex_line {
		vertex vertices[2];

		vertex& get_vert(int i) { return vertices[i]; }
	};

	struct special {
		vec2 v1;
		// float v2;
	};

	typedef std::vector<vertex_triangle> vertex_triangle_buffer;
	typedef std::vector<vertex_line> vertex_line_buffer;
	typedef std::vector<special> special_buffer;
}
