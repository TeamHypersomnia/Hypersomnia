#pragma once
#include "augs/graphics/rgba.h"
#include "augs/math/vec2.h"

enum class vertex_attribute {
	position,
	texcoord,
	color,
	special
};

namespace augs {
	struct atlas_entry;

	struct vertex {
		// GEN INTROSPECTOR struct augs::vertex
		vec2 pos;
		vec2 texcoord;
		rgba color;
		// END GEN INTROSPECTOR

		void set_texcoord(vec2, const augs::atlas_entry& tex);
	};

	struct vertex_triangle {
		std::array<vertex, 3> vertices;

		vertex& get_vert(int i) { return vertices[i]; }
	};

	struct vertex_line {
		vertex vertices[2];

		vertex& get_vert(int i) { return vertices[i]; }
	};

	struct special {
		vec2 v1;
		vec2 v2;
		// float v2;
	};

	using vertex_triangle_buffer = std::vector<vertex_triangle>;
	using vertex_line_buffer = std::vector<vertex_line>;
	using special_buffer = std::vector<special>;
}