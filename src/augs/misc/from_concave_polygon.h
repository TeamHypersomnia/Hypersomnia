#pragma once
#include "3rdparty/polypartition/src/polypartition.h"
#include "augs/graphics/vertex.h"
#include "augs/drawing/polygon.h"
#include "augs/misc/convex_partitioned_shape.h"
#include "3rdparty/Box2D/Common/b2Settings.h"

namespace augs {
	template <
		class T, 
		std::size_t vertex_count, 
		std::size_t index_count
	>
	void refresh_convex_partitioning(
		basic_convex_partitioned_shape<T, vertex_count, index_count>& poly
	) {
		const auto& verts = poly.source_polygon;

		auto& pc = poly.convex_partition;
		pc.clear();

		if (verts.empty()) {
			return;
		}

		const auto partition_result = [&]() {
			std::list<TPPLPoly> output;
			std::list<TPPLPoly> input;

			TPPLPoly subject_poly;
			subject_poly.Init(static_cast<long>(verts.size()));
			subject_poly.SetHole(false);

			for (std::size_t i = 0; i < verts.size(); ++i) {
				const auto ii = static_cast<int>(i);

				auto& in_v = subject_poly[ii];

				in_v.x = verts[i].x;
				in_v.y = verts[i].y;
				in_v.id = ii;
			}

			input.push_back(subject_poly);

			TPPLPartition partition;
			partition.ConvexPartition_HM(&input, &output);

			return output;
		}();

		if (partition_result.size() == 1 && (*partition_result.begin()).GetNumPoints() <= b2_maxPolygonVertices) {
			return;
		}

		using I = typename remove_cref<decltype(pc)>::value_type;

		for (const auto& out_poly : partition_result) {
			auto add_sequence = [&pc, &out_poly](const auto first, const auto num) {
				auto add = [&](auto idx) {
					const auto& out_vertex = out_poly.GetPoint(idx);
					const auto new_id = static_cast<I>(out_vertex.id);
					pc.push_back(new_id);
				};

				{
					const auto how_many_more_can_fit = static_cast<int>(pc.max_size() - pc.size());

					if (how_many_more_can_fit < num + 2) {
						return;
					}
				}

				add(0);

				for (long i = first; i < first + num; ++i) {
					add(i);
				}

				add(0);
			};

			const auto n = out_poly.GetNumPoints();

			long start = 1;

			while (start + 1 < n) {
				const auto how_many_left_to_add = n - start;
				const auto max_at_once = long(b2_maxPolygonVertices) - 1;
				const auto how_many_now = std::min(max_at_once, how_many_left_to_add);

				add_sequence(start, how_many_now);
				start += how_many_now - 1;
			}
		}
	}

	template <class P>
	void fix_polygon_winding(P& polygon) {
		std::size_t i1;
		std::size_t	i2;

		float area = 0;
		const auto& vs = polygon;

		for (i1 = 0; i1 < vs.size(); i1++) {
			i2 = i1 + 1;

			if (i2 == vs.size()) {
				i2 = 0;
			}

			area += vs[i1].cross(vs[i2]);
		}

		/* ensure proper winding */
		if (area < 0) {
			std::reverse(polygon.begin(), polygon.end());
		}
	}

	template <std::size_t vertex_count, std::size_t index_count>
	void from_concave_polygon(
		polygon<vertex_count, index_count>& poly,
	   	std::vector<vertex> polygon
	) {
		if (polygon.empty()) {
			return;
		}

		TPPLPoly inpoly;
		std::list<TPPLPoly> out_tris;

		TPPLPoly subject_poly;
		inpoly.Init(static_cast<long>(polygon.size()));
		inpoly.SetHole(false);

		auto& vertices = poly.vertices;

		vertices.reserve(vertices.size() + polygon.size());

		for (std::size_t i = 0; i < polygon.size(); ++i) {
			vertices.push_back(polygon[i]);
		}

		for (std::size_t i = 0; i < polygon.size(); ++i) {
			vec2 p(polygon[i].pos);
			inpoly[static_cast<int>(i)].x = p.x;
			inpoly[static_cast<int>(i)].y = -p.y;
			inpoly[static_cast<int>(i)].id = static_cast<int>(i);
		}

		TPPLPartition partition;
		partition.Triangulate_EC(&inpoly, &out_tris);

		for (auto& out : out_tris) {
			for (int i = 0; i < 3; ++i) {
				const auto& new_tri_point = out.GetPoint(i);
				poly.triangulation_indices.push_back(static_cast<unsigned>(new_tri_point.id));
			}
		}
	}
}
