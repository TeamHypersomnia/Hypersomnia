#pragma once

#include "math/vec2.h"

#include "graphics/pixel.h"
#include "graphics/vertex.h"
#include "../assets/texture.h"

namespace augs {
	class texture;

	typedef std::vector<vec2> basic_polygon;
}

namespace shared {
	class drawing_state;
}

namespace components {
	class polygon {
	public:
		enum uv_mapping_mode {
			OVERLAY,
			STRETCH
		};

		void automatically_map_uv(assets::texture_id, unsigned uv_mapping_mode);

		/* binding facility */
		struct concave {
			std::vector<vertex> vertices;
			void add_vertex(const vertex& v);
		};

		std::vector<vertex> model;

		/* the "model" is already triangulated so we need to preserve original model vertex data if for example
		we want to form a physics body from this object
		*/
		augs::basic_polygon original_model;

		std::vector<int> indices;

		void set_color(pixel_32 col);

		int get_vertex_count() const {
			return model.size();
		}

		vertex& get_vertex(int i) {
			return model[i];
		}

		/* construct a set of convex polygons from a potentially concave polygon */
		void add_concave(const concave&);
		//void add_concave_coords(const std::vector<vec2>&);
		//void add_convex(const std::vector<vec2>&);

		void draw(shared::drawing_state&);

		std::vector<vec2> get_vertices();
	};
}