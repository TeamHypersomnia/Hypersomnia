#include "convex_partitioned_shape.h"
#include <Box2D/Box2D.h>

#include "3rdparty/polypartition/src/polypartition.h"

#include "game/components/polygon_component.h"
#include "game/components/physics_component.h"
#include "game/components/sprite_component.h"

#include "game/resources/manager.h"
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

void convex_partitioned_shape::mult_vertices(const vec2 mult) {
	for (auto& c : convex_polys) {
		for (auto& v : c.vertices) {
			v *= mult;
		}

		std::reverse(c.vertices.begin(), c.vertices.end());
	}

#if ENABLE_POLYGONIZATION
	for (auto& v : debug_original) {
		v *= mult;
	}
#endif
}

void convex_partitioned_shape::from_renderable(const const_entity_handle handle) {
	if (handle.has<components::sprite>()) {
		from_sprite(handle.get<components::sprite>(), true);
	}
	if (handle.has<components::polygon>()) {
		ensure(false);
	}
}

void convex_partitioned_shape::from_sprite(const components::sprite& sprite, const bool polygonize_sprite) {
	auto& polygonized_sprite_verts = resource_manager.find(sprite.tex)->polygonized;
	auto& image_to_polygonize = resource_manager.find(sprite.tex)->img;

	if (polygonized_sprite_verts.size() > 0 && polygonize_sprite) {
		auto image_size = image_to_polygonize.get_size();
		vec2 polygonized_size = vec2i(image_size.w, image_size.h);

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

#if ENABLE_POLYGONIZATION
		debug_original = new_concave;
#endif
		add_concave_polygon(new_concave);

		mult_vertices(vec2(1, -1));
	}
	else {
		auto rect_size = sprite.size;

		b2PolygonShape shape;
		shape.SetAsBox(static_cast<float>(rect_size.x) / 2.f * PIXELS_TO_METERSf, static_cast<float>(rect_size.y) / 2.f * PIXELS_TO_METERSf);

		convex_poly new_convex_polygon;

		for (int i = 0; i < shape.GetVertexCount(); ++i)
			new_convex_polygon.vertices.push_back(vec2(shape.GetVertex(i))*METERS_TO_PIXELSf);

		add_convex_polygon(new_convex_polygon);
	}

	// TODO: remove the visual components server-side
}

void convex_partitioned_shape::add_concave_polygon(const std::vector <vec2> &verts) {
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
