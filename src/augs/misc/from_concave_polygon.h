#pragma once
#include "3rdparty/polypartition/src/polypartition.h"
#include "augs/graphics/vertex.h"
#include "augs/drawing/polygon.h"
#include "augs/misc/convex_partitioned_shape.h"

namespace augs {
	template <
		class T, 
		std::size_t vertex_count, 
		std::size_t index_count
	>
	void refresh_convex_partitioning(
		basic_convex_partitioned_shape<T, vertex_count, index_count>& poly
	) {
		const auto& verts = poly.original_poly;

		const auto partition_result = [&]() {
			std::list<TPPLPoly> output;
			std::list<TPPLPoly> input;

			TPPLPoly subject_poly;
			subject_poly.Init(static_cast<long>(verts.size()));
			subject_poly.SetHole(false);

			for (std::size_t i = 0; i < verts.size(); ++i) {
				vec2 p(verts[i]);
				subject_poly[static_cast<int>(i)].x = p.x;
				subject_poly[static_cast<int>(i)].y = -p.y;
				subject_poly[static_cast<int>(i)].id = static_cast<int>(i);
			}

			input.push_back(subject_poly);

			TPPLPartition partition;
			partition.ConvexPartition_HM(&input, &output);

			return output;
		}();

		auto& pc = poly.convex_partition;
		pc.clear();

		if (partition_result.size() == 1) {
			return;
		}

		for (const auto& out_poly : partition_result) {
			{
				const auto how_many_more_can_fit = static_cast<int>(pc.max_size() - pc.size());

				if (how_many_more_can_fit < out_poly.GetNumPoints() + 1) {
					break;
				}
			}

			using I = typename remove_cref<decltype(pc)>::value_type;

			for (long i = 0; i < out_poly.GetNumPoints(); ++i) {
				const auto& out_vertex = out_poly.GetPoint(i);
				pc.push_back(static_cast<I>(out_vertex.id));
			}

			pc.push_back(static_cast<I>(out_poly.GetPoint(0).id));
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

		std::size_t i1;
		std::size_t	i2;

		float area = 0;
		const auto& vs = polygon;

		for (i1 = 0; i1 < vs.size(); i1++) {
			i2 = i1 + 1;

			if (i2 == vs.size()) {
				i2 = 0;
			}

			area += vs[i1].pos.cross(vs[i2].pos);
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

		auto& vertices = poly.vertices;

		vertices.reserve(vertices.size() + polygon.size());

		for (std::size_t i = 0; i < polygon.size(); ++i) {
			vertices.push_back(polygon[i]);
			//original_polygon.push_back(polygon[i].pos);
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
