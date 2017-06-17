#pragma once
#include "game/container_sizes.h"
#include "augs/misc/constant_size_vector.h"

#include "augs/math/vec2.h"

#include "augs/graphics/rgba.h"
#include "augs/graphics/vertex.h"
#include "game/assets/game_image_id.h"
#include "game/detail/basic_renderable_drawing_input.h"

#include "augs/zeroed_pod.h"

namespace augs {
	struct texture_atlas_entry;
}

struct convex_partitioned_shape;

namespace components {
	struct polygon {
		enum class uv_mapping_mode {
			STRETCH,

			COUNT
		};

		struct drawing_input : basic_renderable_drawing_input {
			using basic_renderable_drawing_input::basic_renderable_drawing_input;

			double global_time_seconds = 0.0;
			void set_global_time_seconds(const double);
		};

		// GEN INTROSPECTOR struct components::polygon
		assets::game_image_id texture_map = assets::game_image_id::INVALID;

		augs::constant_size_vector<augs::vertex, RENDERING_POLYGON_TRIANGULATED_VERTEX_COUNT> vertices;
		augs::constant_size_vector<zeroed_pod<unsigned>, RENDERING_POLYGON_INDEX_COUNT> triangulation_indices;

		// END GEN INTROSPECTOR
		
		void automatically_map_uv(const uv_mapping_mode);
		void add_vertices_from(const convex_partitioned_shape&);

		/* triangulates input */
		void add_concave_polygon(std::vector<augs::vertex>);
		void add_triangle(const augs::vertex_triangle&);

		void set_color(const rgba col);

		size_t get_vertex_count() const {
			return vertices.size();
		}

		augs::vertex& get_vertex(const std::size_t i) {
			return vertices[i];
		}

		void draw(const drawing_input&) const;

		std::vector<vec2> get_vertices() const;
		ltrb get_aabb(const components::transform) const;
	};
}