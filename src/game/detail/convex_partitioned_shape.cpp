#include "convex_partitioned_shape.h"
#include <Box2D/Box2D.h>

#include "3rdparty/polypartition/src/polypartition.h"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"

void convex_partitioned_shape::add_convex_polygon(const convex_poly& verts) {
	convex_polys.push_back(verts);
}

void convex_partitioned_shape::offset_vertices(const components::transform transform) {
	for (auto& c : convex_polys) {
		for (auto& v : c.vertices) {
			v.rotate(transform.rotation, vec2(0, 0));
			v += transform.pos;
		}
	}
}

void convex_partitioned_shape::scale(const vec2 mult) {
	for (auto& c : convex_polys) {
		for (auto& v : c.vertices) {
			v *= mult;
		}
	}
}

void convex_partitioned_shape::add_concave_polygon(const std::vector<vec2> &verts) {
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

		const unsigned max_vertices = CONVEX_POLY_VERTEX_COUNT;

		if (new_convex.size() > max_vertices) {
			unsigned first = 1;

			while (first + max_vertices - 2 < new_convex.size() - 1) {
				convex_poly new_poly;
				new_poly.vertices.push_back(new_convex[0]);
				//new_poly.insert(new_poly.end(), new_convex.begin() + first, new_convex.begin() + first + max_vertices - 2 + 1);
				auto s = new_convex.begin() + first;
				auto e = new_convex.begin() + first + max_vertices - 2 + 1;
				
				while (s != e)
					new_poly.vertices.push_back(*s++);

				convex_polys.push_back(new_poly);
				first += max_vertices - 2;
			}

			convex_poly last_poly;
			last_poly.vertices.push_back(new_convex[0]);
			last_poly.vertices.push_back(new_convex[first]);

			// last_poly.insert(last_poly.end(), new_convex.begin() + first, new_convex.end());
			auto s = new_convex.begin() + first;
			auto e = new_convex.end();

			while (s != e)
				last_poly.vertices.push_back(*s++);

			convex_polys.push_back(last_poly);
		}
		else {
			convex_poly new_poly;
			new_poly.vertices.assign(new_convex.begin(), new_convex.end());
			convex_polys.push_back(new_poly);
		}
	}
}
