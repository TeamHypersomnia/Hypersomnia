#pragma once
#include "game/container_sizes.h"
#include "augs/misc/constant_size_vector.h"

#include "augs/math/vec2.h"

#include "augs/graphics/pixel.h"
#include "augs/graphics/vertex.h"
#include "game/assets/texture_id.h"
#include "transform_component.h"

#include "zeroed_pod.h"

namespace augs {
	class texture;
}

namespace components {
	struct polygon {
		enum uv_mapping_mode {
			OVERLAY,
			STRETCH
		};

		struct drawing_input : vertex_triangle_buffer_reference {
			using vertex_triangle_buffer_reference::vertex_triangle_buffer_reference;

			components::transform renderable_transform;
			components::transform camera_transform;
			vec2 visible_world_area;

			augs::rgba colorize = augs::white;
		};

		/* the polygon as it was originally, so possibly concave
		it is later triangulated for rendering and divided into convex polygons for physics */
		augs::constant_size_vector<vec2, RENDERING_POLYGON_VERTEX_COUNT> original_polygon;

		/* triangulated version of original_polygon, ready to be rendered triangle-by-triangle */
		augs::constant_size_vector<vertex, RENDERING_POLYGON_TRIANGULATED_VERTEX_COUNT> triangulated_polygon;

		/* indices used in glDrawElements */
		augs::constant_size_vector<zeroed_pod<unsigned>, RENDERING_POLYGON_INDEX_COUNT> indices;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(original_polygon),

				CEREAL_NVP(triangulated_polygon),

				CEREAL_NVP(indices)
			);
		}
		
		void automatically_map_uv(assets::texture_id, unsigned uv_mapping_mode);

		/* construct a set of convex polygons from a potentially concave polygon */
		void add_polygon_vertices(std::vector<vertex>);

		void set_color(rgba col);

		size_t get_vertex_count() const {
			return triangulated_polygon.size();
		}

		vertex& get_vertex(size_t i) {
			return triangulated_polygon[i];
		}

		void draw(const drawing_input&) const;

		std::vector<vec2> get_vertices() const;
		rects::ltrb<float> get_aabb(components::transform) const;
	};
}