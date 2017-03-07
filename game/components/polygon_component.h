#pragma once
#include "game/container_sizes.h"
#include "augs/misc/constant_size_vector.h"

#include "augs/math/vec2.h"

#include "augs/graphics/pixel.h"
#include "augs/graphics/vertex.h"
#include "game/assets/game_image_id.h"
#include "game/detail/basic_renderable_drawing_input.h"

#include "zeroed_pod.h"

namespace augs {
	struct texture_atlas_entry;
}

namespace components {
	struct polygon {
		enum class uv_mapping_mode {
			OVERLAY,
			STRETCH
		};

		struct drawing_input : basic_renderable_drawing_input {
			using basic_renderable_drawing_input::basic_renderable_drawing_input;

			float global_time_seconds = 0.f;
			void set_global_time_seconds(const float);
		};

		// GEN INTROSPECTOR components::polygon
		assets::game_image_id center_neon_map = assets::game_image_id::INVALID;
		augs::constant_size_vector<augs::vertex, RENDERING_POLYGON_TRIANGULATED_VERTEX_COUNT> vertices;
		augs::constant_size_vector<zeroed_pod<unsigned>, RENDERING_POLYGON_INDEX_COUNT> triangulation_indices;

		// END GEN INTROSPECTOR

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(original_polygon),

				CEREAL_NVP(vertices),

				CEREAL_NVP(triangulation_indices)
			);
		}
		
		void automatically_map_uv(const assets::game_image_id, const uv_mapping_mode);
		void from_polygonized_texture(const assets::game_image_id);

		/* triangulates input */
		void add_concave_polygon(std::vector<augs::vertex>);
		void add_triangle(const augs::vertex_triangle&);

		void set_color(rgba col);

		size_t get_vertex_count() const {
			return vertices.size();
		}

		augs::vertex& get_vertex(size_t i) {
			return vertices[i];
		}

		void draw(const drawing_input&) const;

		std::vector<vec2> get_vertices() const;
		ltrb get_aabb(components::transform) const;
	};
}