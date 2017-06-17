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
	struct texture_atlas_entry;

	struct vertex {
		// GEN INTROSPECTOR struct augs::vertex
		vec2 pos;
		vec2 texcoord;
		rgba color;
		// END GEN INTROSPECTOR

		void set_texcoord(vec2, const augs::texture_atlas_entry& tex);
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
		vec2 v2;
		// float v2;
	};

	typedef std::vector<vertex_triangle> vertex_triangle_buffer;
	typedef std::vector<vertex_line> vertex_line_buffer;
	typedef std::vector<special> special_buffer;
}

struct vertex_triangle_buffer_reference {
	augs::vertex_triangle_buffer& target_buffer;
	vertex_triangle_buffer_reference(augs::vertex_triangle_buffer&);
};
