#pragma once
#include "augs/templates/algorithm_templates.h"
#include "augs/math/vec2.h"
#include "augs/misc/constant_size_vector.h"

#include "augs/graphics/rgba.h"
#include "augs/graphics/vertex.h"

#include "augs/drawing/sprite.h"

struct polygon_drawing_input;

enum class uv_mapping_mode {
	STRETCH
};

template <class vertex_container>
void map_uv(vertex_container& vertices, const uv_mapping_mode mapping_mode) {
	static_assert(
		std::is_same_v<typename vertex_container::value_type, augs::vertex>,
		"Mapping only works for containers of augs::vertex!"	
	);

	if (vertices.empty()) {
		return;
	}

	using vc = const augs::vertex&;

	const auto x_pred = [](vc a, vc b) { return a.pos.x < b.pos.x; };
	const auto y_pred = [](vc a, vc b) { return a.pos.y < b.pos.y; };

	const auto lower = vec2i(
		static_cast<int>(minimum_of(vertices, x_pred).pos.x),
		static_cast<int>(minimum_of(vertices, y_pred).pos.y)
	);

	const auto upper = vec2i(
		static_cast<int>(maximum_of(vertices, x_pred).pos.x),
		static_cast<int>(maximum_of(vertices, y_pred).pos.y)
	);

	if (mapping_mode == uv_mapping_mode::STRETCH) {
		for (auto& v : vertices) {
			v.texcoord = vec2(
				(v.pos.x - lower.x) / (upper.x - lower.x),
				(v.pos.y - lower.y) / (upper.y - lower.y)
			);
		}
	}
	else {
		ensure(false && "Unknown polygon uv mapping mode!");
	}
}

namespace augs {
	template <std::size_t vertex_count, std::size_t index_count>
	struct polygon {
		using drawing_input = polygon_drawing_input;

		// The texture coordinates in vertices are in the local 0.0 - 1.0 space of the texture,
		// and are remapped to global atlas coordinates per every draw,
		// so there is no need to exclude them from the significant state.

		// GEN INTROSPECTOR struct augs::polygon std::size_t vertex_count std::size_t index_count
		constant_size_vector<vertex, vertex_count> vertices = {};
		constant_size_vector<unsigned, index_count> triangulation_indices = {};
		// END GEN INTROSPECTOR

		size_t get_vertex_count() const {
			return vertices.size();
		}

		void set_color(const rgba col) {
			for (auto& v : vertices) {
				v.color = col;
			}
		}

		std::vector<vec2> get_vertices() const {
			std::vector<vec2> out;

			for (auto& v : vertices) {
				out.push_back(v.pos);
			}

			return out;
		}

		ltrb get_aabb(const transformr transform) const {
			auto model_transformed = vertices;

			for (auto& v : model_transformed) {
				v.pos.rotate(transform.rotation);
				v.pos += transform.pos;
			}

			return augs::get_aabb(
				model_transformed,
				[](const vertex& p) { return p.pos.x; },
				[](const vertex& p) { return p.pos.y; }
			);
		}
	};

	template <
		class id_type,
		std::size_t vertex_count,
		std::size_t index_count
	>
	struct polygon_with_id : polygon<vertex_count, index_count> {
		static constexpr bool reinfer_when_tweaking = true;

		using introspect_base = polygon<vertex_count, index_count>;

		// GEN INTROSPECTOR struct augs::polygon_with_id class id_type std::size_t vertex_count std::size_t index_count
		id_type texture_map_id;
		// END GEN INTROSPECTOR

		using base = polygon<vertex_count, index_count>;
		using typename base::drawing_input;
	};
}