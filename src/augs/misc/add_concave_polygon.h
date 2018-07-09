#pragma once
#include "3rdparty/polypartition/src/polypartition.h"
#include "augs/graphics/vertex.h"
#include "augs/drawing/polygon.h"
#include "augs/misc/convex_partitioned_shape.h"

namespace augs {
	template <
		class T,
		std::size_t convex_polys_count,
		std::size_t convex_poly_vertex_count,
		class C
	>
	void add_concave_polygon(
		basic_convex_partitioned_shape<T, convex_polys_count, convex_poly_vertex_count>& poly,
		const C& verts
	) {
		std::list<TPPLPoly> inpolys, outpolys;
		TPPLPoly subject_poly;
		subject_poly.Init(static_cast<long>(verts.size()));
		subject_poly.SetHole(false);

		for (std::size_t i = 0; i < verts.size(); ++i) {
			vec2 p(verts[i]);
			subject_poly[static_cast<int>(i)].x = p.x;
			subject_poly[static_cast<int>(i)].y = -p.y;
		}

		inpolys.push_back(subject_poly);

		TPPLPartition partition;
		partition.ConvexPartition_HM(&inpolys, &outpolys);

		for (auto& out : outpolys) {
			std::vector <vec2> new_convex;

			for (long j = 0; j < out.GetNumPoints(); ++j) {
				new_convex.push_back(vec2(static_cast<float>(out[j].x), static_cast<float>(-out[j].y)));
			}

			std::reverse(new_convex.begin(), new_convex.end());
			poly.add_convex_polygon(new_convex);
		}
	}

	template <std::size_t vertex_count, std::size_t index_count>
	void add_concave_polygon(
		polygon<vertex_count, index_count>& poly,
	   	std::vector<vertex> polygon
	) {
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

		auto& vertices = poly.vertices;

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
						poly.triangulation_indices.push_back(static_cast<unsigned>(j));
						break;
					}
				}
			}
		}
	}
}
