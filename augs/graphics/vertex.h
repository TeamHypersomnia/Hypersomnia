#pragma once
#include "augs/graphics/pixel.h"
#include "augs/math/vec2.h"

enum class vertex_attribute {
	position,
	texcoord,
	color,
	special
};

namespace augs {
	class texture;

	struct vertex {
		vec2 pos;
		vec2 texcoord;
		rgba color;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(pos),
				CEREAL_NVP(texcoord),
				CEREAL_NVP(color)
			);
		}

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
