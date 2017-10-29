#pragma once
#include "augs/drawing/polygon.h"

#include "augs/math/vec2.h"
#include "augs/math/transform.h"

#include "augs/misc/constant_size_vector.h"

template <
	class T,
	std::size_t convex_polys_count,
	std::size_t convex_poly_vertex_count
>
struct basic_convex_partitioned_shape {
	using vec2 = basic_vec2<T>;
	using transform = basic_transform<T>;

	using convex_poly = augs::constant_size_vector<vec2, convex_poly_vertex_count>;

	// GEN INTROSPECTOR struct basic_convex_partitioned_shape class T std::size_t convex_polys_count std::size_t convex_poly_vertex_count
	augs::constant_size_vector<convex_poly, convex_polys_count> convex_polys;
	// END GEN INTROSPECTOR

	void offset_vertices(const transform transform) {
		for (auto& c : convex_polys) {
			for (auto& v : c) {
				v.rotate(transform.rotation, vec2(0, 0));
				v += transform.pos;
			}
		}
	}

	void scale(const vec2 mult) {
		for (auto& c : convex_polys) {
			for (auto& v : c) {
				v *= mult;
			}
		}
	}

	void add_convex_polygon(const convex_poly& poly) {
		convex_polys.push_back(poly);
	}

	template <class C>
	void add_concave_polygon(const C& verts) {
		std::list<TPPLPoly> inpolys, outpolys;
		TPPLPoly subject_poly;
		subject_poly.Init(verts.size());
		subject_poly.SetHole(false);

		for (size_t i = 0; i < verts.size(); ++i) {
			vec2 p(verts[i]);
			subject_poly[i].x = p.x;
			subject_poly[i].y = -p.y;
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

			auto first_v = new_convex[0];

			const unsigned max_vertices = convex_poly_vertex_count;

			if (new_convex.size() > max_vertices) {
				unsigned first = 1;

				while (first + max_vertices - 2 < new_convex.size() - 1) {
					convex_poly new_poly;
					new_poly.push_back(new_convex[0]);
					//new_poly.insert(new_poly.end(), new_convex.begin() + first, new_convex.begin() + first + max_vertices - 2 + 1);
					auto s = new_convex.begin() + first;
					auto e = new_convex.begin() + first + max_vertices - 2 + 1;
					
					while (s != e)
						new_poly.push_back(*s++);

					convex_polys.push_back(new_poly);
					first += max_vertices - 2;
				}

				convex_poly last_poly;
				last_poly.push_back(new_convex[0]);
				last_poly.push_back(new_convex[first]);

				// last_poly.insert(last_poly.end(), new_convex.begin() + first, new_convex.end());
				auto s = new_convex.begin() + first;
				auto e = new_convex.end();

				while (s != e)
					last_poly.push_back(*s++);

				convex_polys.push_back(last_poly);
			}
			else {
				convex_poly new_poly;
				new_poly.assign(new_convex.begin(), new_convex.end());
				convex_polys.push_back(new_poly);
			}
		}
	}
};