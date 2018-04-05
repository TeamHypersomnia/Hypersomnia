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
	using poly_vector_type = augs::constant_size_vector<convex_poly, convex_polys_count>;

	// GEN INTROSPECTOR struct basic_convex_partitioned_shape class T std::size_t convex_polys_count std::size_t convex_poly_vertex_count
	poly_vector_type convex_polys = {};
	// END GEN INTROSPECTOR

	void make_box(const vec2 size) {
		auto hx = size.x / 2;
		auto hy = size.y / 2;

		convex_poly new_poly;
		new_poly.resize(4);
		new_poly[0].set(-hx, -hy);
		new_poly[1].set( hx, -hy);
		new_poly[2].set( hx,  hy);
		new_poly[3].set(-hx,  hy);
		convex_polys.push_back(new_poly);
	}

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

	template <class C>
	void add_convex_polygon(const C& new_convex, const unsigned from) {
		const unsigned max_vertices = convex_poly_vertex_count;

		const auto remaining = new_convex.size() - from;

		if (remaining + 1 > max_vertices) {
			convex_poly new_poly;

			new_poly.push_back(new_convex[0]);

			new_poly.insert(
				new_poly.end(),
				new_convex.begin() + from, 
				new_convex.begin() + from + max_vertices
			);

			convex_polys.push_back(new_poly);

			const auto next_starting = from + max_vertices - 2;
			add_convex_polygon(new_convex, next_starting);
		}
		else {
			convex_poly new_poly;

			new_poly.push_back(new_convex[0]);

			new_poly.insert(
				new_poly.end(),
				new_convex.begin() + from, 
				new_convex.end()
			);

			convex_polys.push_back(new_poly);
		}
	}

	template <class C>
	void add_convex_polygon(const C& new_convex) {
		const unsigned max_vertices = convex_poly_vertex_count;

		if (new_convex.size() > max_vertices) {
			convex_poly new_poly;
			new_poly.assign(new_convex.begin(), new_convex.begin() + max_vertices);
			convex_polys.push_back(new_poly);

			const auto next_starting = max_vertices - 1;
			add_convex_polygon(new_convex, next_starting);
		}
		else {
			convex_poly new_poly;
			new_poly.assign(new_convex.begin(), new_convex.end());
			convex_polys.push_back(new_poly);
		}
	}

	template <class C>
	void add_concave_polygon(const C& verts) {
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
			add_convex_polygon(new_convex);
		}
	}
};