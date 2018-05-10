#pragma once
#include "3rdparty/polypartition/src/polypartition.h"

#include "augs/templates/algorithm_templates.h"
#include "augs/math/vec2.h"
#include "augs/misc/constant_size_vector.h"

#include "augs/graphics/rgba.h"
#include "augs/graphics/vertex.h"

#include "augs/drawing/sprite.h"

#include "augs/texture_atlas/atlas_entry.h"

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
		struct drawing_input : drawing_input_base {
			using drawing_input_base::drawing_input_base;

			double global_time_seconds = 0.0;
		};

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

		template <class C>
		void add_convex_polygons(const C& convexes) {
			for (const auto& c : convexes) {
				std::vector<vertex> new_concave;

				for (auto v : c) {
					vertex new_v;
					new_v.pos = v;
					new_concave.push_back(new_v);
				}

				add_concave_polygon(new_concave);
			}

			map_uv(vertices, uv_mapping_mode::STRETCH);
		}

		void add_concave_polygon(std::vector<vertex> polygon) {
			if (polygon.empty()) {
				return;
			}

			size_t i1, i2;

			float area = 0;
			const auto& vs = polygon;

			for (i1 = 0; i1 < vs.size(); i1++) {
				i2 = i1 + 1;
				if (i2 == vs.size()) i2 = 0;
				area += vs[i1].pos.x * vs[i2].pos.y - vs[i1].pos.y * vs[i2].pos.x;
			}

			/* ensure proper winding */
			if (area > 0) {
				std::reverse(polygon.begin(), polygon.end());
			}

			TPPLPoly inpoly;
			std::list<TPPLPoly> out_tris;

			TPPLPoly subject_poly;
			inpoly.Init(static_cast<long>(polygon.size()));
			inpoly.SetHole(false);

			vertices.reserve(vertices.size() + polygon.size());

			const auto offset = vertices.size();
			
			for (std::size_t i = 0; i < polygon.size(); ++i) {
				vertices.push_back(polygon[i]);
				//original_polygon.push_back(polygon[i].pos);
			}

			for (std::size_t i = 0; i < polygon.size(); ++i) {
				vec2 p(polygon[i].pos);
				inpoly[static_cast<int>(i)].x = p.x;
				inpoly[static_cast<int>(i)].y = -p.y;
			}

			TPPLPartition partition;
			partition.Triangulate_EC(&inpoly, &out_tris);

			for (auto& out : out_tris) {
				for (int i = 0; i < 3; ++i) {
					auto new_tri_point = out.GetPoint(i);

					for (std::size_t j = offset; j < polygon.size(); ++j) {
						if (polygon[j].pos.compare(vec2(static_cast<float>(new_tri_point.x), static_cast<float>(-new_tri_point.y)), 1.f)) {
							triangulation_indices.push_back(static_cast<unsigned>(j));
							break;
						}
					}
				}
			}
		}

		std::vector<vec2> get_vertices() const {
			std::vector<vec2> out;

			for (auto& v : vertices) {
				out.push_back(v.pos);
			}

			return out;
		}
		
		void draw(
			const augs::drawer output,
			const atlas_entry texture,
			const transform target_transform
		) const {
			vertex_triangle new_tri;

			auto model_transformed = vertices;

			for (auto& v : model_transformed) {
				if (std::abs(target_transform.rotation) > 0.f) {
					v.pos.rotate(target_transform.rotation, vec2(0, 0));
				}

				v.pos += target_transform.pos;

				v.pos.x = static_cast<float>(static_cast<int>(v.pos.x));
				v.pos.y = static_cast<float>(static_cast<int>(v.pos.y));

				v.set_texcoord(v.texcoord, texture);
			}

			for (std::size_t i = 0; i < triangulation_indices.size(); i += 3) {
				new_tri.vertices[0] = model_transformed[triangulation_indices[i]];
				new_tri.vertices[1] = model_transformed[triangulation_indices[i + 1]];
				new_tri.vertices[2] = model_transformed[triangulation_indices[i + 2]];

				output.push(new_tri);
			}
		}
		
		ltrb get_aabb(const transform transform) const {
			auto model_transformed = vertices;

			for (auto& v : model_transformed) {
				v.pos.rotate(transform.rotation, vec2(0, 0));
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
		using base::draw;
		using typename base::drawing_input;

		template <class M>
		void draw(
			const M& manager,
			const drawing_input& in
		) const {
			const auto texture = manager.at(texture_map_id);

			if (in.use_neon_map) {
				using sprite_type = sprite<id_type>;

				auto neon_in = typename sprite_type::drawing_input{ in.output };

				neon_in.renderable_transform = in.renderable_transform;
				neon_in.colorize = in.colorize;

				neon_in.global_time_seconds = in.global_time_seconds;
				neon_in.use_neon_map = true;

				sprite_type neon_sprite;
				neon_sprite.set(texture_map_id, texture.get_original_size());

				neon_sprite.draw(manager, neon_in);

				return;
			}

			draw(
				in.output,
				texture,
				in.renderable_transform
			);
		}
	};
}