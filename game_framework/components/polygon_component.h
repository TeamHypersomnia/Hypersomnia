#pragma once

#include "math/vec2.h"

#include "graphics/pixel.h"
#include "graphics/vertex.h"
#include "../assets/texture.h"
#include "transform_component.h"

namespace augs {
	class texture;
}

namespace shared {
	class drawing_state;
}

namespace components {
	struct polygon {
		enum uv_mapping_mode {
			OVERLAY,
			STRETCH
		};

		void automatically_map_uv(assets::texture_id, unsigned uv_mapping_mode);

		/* the polygon as it was originally, so possibly concave
		it is later triangulated for rendering and divided into convex polygons for physics */
		std::vector<vec2> original_polygon;

		/* triangulated version of original_polygon, ready to be rendered triangle-by-triangle */
		std::vector<vertex> triangulated_polygon;

		/* indices used in glDrawElements */
		std::vector<int> indices;

		/* construct a set of convex polygons from a potentially concave polygon */
		void add_polygon_vertices(std::vector<vertex>);

		void set_color(rgba col);

		int get_vertex_count() const {
			return triangulated_polygon.size();
		}

		vertex& get_vertex(int i) {
			return triangulated_polygon[i];
		}

		void draw(shared::drawing_state&);

		std::vector<vec2> get_vertices();
		rects::ltrb<float> get_aabb(components::transform);
	};
}