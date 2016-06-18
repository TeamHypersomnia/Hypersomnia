#include "convex_partitioned_shape.h"
#include <Box2D/Box2D.h>

#include "3rdparty/polypartition/polypartition.h"

#include "game/components/polygon_component.h"
#include "game/components/physics_component.h"
#include "game/components/sprite_component.h"

#include "game/resources/manager.h"

void convex_partitioned_shape::add_convex_polygon(const std::vector <vec2>& verts) {
	convex_polys.push_back(verts);
}

void convex_partitioned_shape::offset_vertices(components::transform transform) {
	for (auto& c : convex_polys) {
		for (auto& v : c) {
			v.rotate(transform.rotation, vec2(0, 0));
			v += transform.pos;
		}
	}
}

void convex_partitioned_shape::mult_vertices(vec2 mult) {
	for (auto& c : convex_polys) {
		for (auto& v : c) {
			v *= mult;
		}

		std::reverse(c.begin(), c.end());
	}

	for (auto& v : debug_original) {
		v *= mult;
	}
}

void convex_partitioned_shape::from_sprite(const components::sprite& sprite, bool polygonize_sprite) {
	auto& polygonized_sprite_verts = resource_manager.find(sprite.tex)->polygonized;
	auto& image_to_polygonize = resource_manager.find(sprite.tex)->img;

	if (polygonized_sprite_verts.size() > 0 && polygonize_sprite) {
		type = POLYGON;

		auto image_size = image_to_polygonize.get_size();
		vec2 polygonized_size(image_size.w, image_size.h);

		std::vector<vec2> new_concave;

		for (auto v : polygonized_sprite_verts) {
			vec2 new_v = v;
			vec2 scale = sprite.size / polygonized_size;

			new_v *= scale;
			new_v.y = -new_v.y;
			new_concave.push_back(new_v);
		}

		auto origin = augs::get_aabb(new_concave).center();

		for (auto& v : new_concave)
			v -= origin;

		debug_original = new_concave;
		add_concave_polygon(new_concave);

		mult_vertices(vec2(1, -1));
	}
	else {
		type = RECT;
		rect_size = sprite.size;

		b2PolygonShape shape;
		shape.SetAsBox(static_cast<float>(rect_size.x) / 2.f * PIXELS_TO_METERSf, static_cast<float>(rect_size.y) / 2.f * PIXELS_TO_METERSf);

		std::vector<vec2> new_convex_polygon;

		for (int i = 0; i < shape.GetVertexCount(); ++i)
			new_convex_polygon.push_back(vec2(shape.GetVertex(i))*METERS_TO_PIXELSf);

		add_convex_polygon(new_convex_polygon);
	}

	// TODO: remove the visual components server-side
}

void convex_partitioned_shape::from_polygon(const components::polygon& poly) {
	type = POLYGON;
	add_concave_polygon(poly.original_polygon);
}

void convex_partitioned_shape::add_concave_polygon(const std::vector <vec2> &verts) {
	list<TPPLPoly> inpolys, outpolys;
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

		const int max_vertices = 8;

		if (new_convex.size() > max_vertices) {
			int first = 1;

			while (first + max_vertices - 2 < new_convex.size() - 1) {
				std::vector<vec2> new_poly;
				new_poly.push_back(new_convex[0]);
				new_poly.insert(new_poly.end(), new_convex.begin() + first, new_convex.begin() + first + max_vertices - 2 + 1);
				convex_polys.push_back(new_poly);
				first += max_vertices - 2;
			}

			std::vector<vec2> last_poly;
			last_poly.push_back(new_convex[0]);
			last_poly.push_back(new_convex[first]);
			last_poly.insert(last_poly.end(), new_convex.begin() + first, new_convex.end());

			convex_polys.push_back(last_poly);
		}
		else
			convex_polys.push_back(new_convex);
	}
}
