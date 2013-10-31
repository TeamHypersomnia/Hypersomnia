#include "stdafx.h"

#include "render_info.h"

#include <algorithm>
#include "entity_system/entity.h"

#include "../../topdown/components/physics_component.h"
#include "../../topdown/components/particle_group_component.h"

#include "utility/sorted_vector.h"
#include "poly2tri/poly2tri.h"

namespace resources {
	void renderable::make_rect(vec2<> pos, vec2<> size, float angle, vec2<> v[4]) {
		vec2<> origin(pos + (size / 2.f));

		v[0] = pos;
		v[1] = pos + vec2<>(size.x, 0.f);
		v[2] = pos + size;
		v[3] = pos + vec2<>(0.f, size.y);

		v[0].rotate(angle, origin);
		v[1].rotate(angle, origin);
		v[2].rotate(angle, origin);
		v[3].rotate(angle, origin);

		v[0] -= size / 2.f;
		v[1] -= size / 2.f;
		v[2] -= size / 2.f;
		v[3] -= size / 2.f;
	}

	sprite::sprite(texture_baker::texture* tex, graphics::pixel_32 color) : tex(tex), color(color) {
		set(tex, color);
	}

	void sprite::set(texture_baker::texture* _tex, graphics::pixel_32 _color) {
		tex = _tex;
		color = _color;

		if (tex)
			size = tex->get_size();
	}

	void sprite::update_size() {
		if (tex)
			size = tex->get_size();
	}

	void sprite::draw(buffer& triangles, const components::transform::state& transform, vec2<> camera_pos) {
		if (tex == nullptr) return;
		vec2<> v[4];
		make_rect(transform.pos - camera_pos, vec2<>(size), transform.rotation, v);

		vertex_triangle t1, t2;
		t1.vertices[0].color = t2.vertices[0].color = color;
		t1.vertices[1].color = t2.vertices[1].color = color;
		t1.vertices[2].color = t2.vertices[2].color = color;

		t1.vertices[0].texcoord = t2.vertices[0].texcoord = vec2<>(0.f, 0.f);
		t2.vertices[1].texcoord = vec2<>(1.f, 0.f);
		t1.vertices[1].texcoord = t2.vertices[2].texcoord = vec2<>(1.f, 1.f);
		t1.vertices[2].texcoord = vec2<>(0.f, 1.f);

		for (int i = 0; i < 3; ++i) {
			tex->get_uv(t1.vertices[i].texcoord);
			tex->get_uv(t2.vertices[i].texcoord);
		}

		t1.vertices[0].position = t2.vertices[0].position = vec2<int>(v[0]);
		t2.vertices[1].position = vec2<int>(v[1]);
		t1.vertices[1].position = t2.vertices[2].position = vec2<int>(v[2]);
		t1.vertices[2].position = vec2<int>(v[3]);

		triangles.emplace_back(t1);
		triangles.emplace_back(t2);
	}

	bool sprite::is_visible(rects::xywh visibility_aabb, const components::transform::state& transform) {
		vec2<> v[4];
		make_rect(transform.pos, vec2<>(size), transform.rotation, v);

		typedef const vec2<>& vc;

		rects::xywh out;
		auto x_pred = [](vc a, vc b){ return a.x < b.x; };
		auto y_pred = [](vc a, vc b){ return a.y < b.y; };

		vec2<int> lower(
			static_cast<int>(std::min_element(v, v + 4, x_pred)->x),
			static_cast<int>(std::min_element(v, v + 4, y_pred)->y)
			);

		vec2<int> upper(
			static_cast<int>(std::max_element(v, v + 4, x_pred)->x),
			static_cast<int>(std::max_element(v, v + 4, y_pred)->y)
			);

		return rects::ltrb(lower.x, lower.y, upper.x, upper.y).hover(visibility_aabb);
	}
	
	bool polygon::is_visible(rects::xywh visibility_aabb, const components::transform::state&) {
		/* perform visibility check! */
		return true;
	}

	//triangle::triangle(const vertex& a, const vertex& b, const vertex& c) {
	//	vertices[0] = a;
	//	vertices[1] = b;
	//	vertices[2] = c;
	//}
	//
	//void triangle::draw(buffer& triangles, const components::transform::state& transform, vec2<> camera_pos) {
	//
	//}
	//
	//bool triangle::is_visible(rects::xywh visibility_aabb, const components::transform::state& transform) {
	//	return true;
	//}

	void polygon::concave::add_vertex(const vertex& v) {
		vertices.push_back(v);
	}

	void polygon::add_concave(const concave& polygon) {
		using namespace p2t;
		std::vector<Point> points;
		std::vector<Point*> input;
		
		points.reserve(polygon.vertices.size());
		input.reserve(polygon.vertices.size());
		model.reserve(model.size() + polygon.vertices.size());

		int offset = model.size();
		for (int i = 0; i < polygon.vertices.size(); ++i) {
			points.push_back(Point(polygon.vertices[i].position.x, polygon.vertices[i].position.y, i + offset));
			model.push_back(polygon.vertices[i]);
		}

		for (auto& v : points) 
			input.push_back(&v);

		/* perform decomposition */		
		CDT cdt(std::move(input));
		cdt.Triangulate();
		auto output = cdt.GetTriangles();

		for (auto& tri : output) {
			indices.push_back(tri->GetPoint(0)->index);
			indices.push_back(tri->GetPoint(1)->index);
			indices.push_back(tri->GetPoint(2)->index);
		}
	}

	//void polygon::add_convex(const std::vector<vertex>& model) {
	//	convex_models.push_back(model);
	//}

	void polygon::draw(buffer& triangles, const components::transform::state& transform, vec2<> camera_pos) {
		vertex_triangle new_tri;
		
		auto model_transformed = model;
		for (auto& v : model_transformed) {
				v.position.rotate(transform.rotation, vec2<>(0, 0));
				v.position += transform.pos - camera_pos;
		}

		for (int i = 0; i < indices.size(); i += 3) {
			new_tri.vertices[0] = model_transformed[indices[i]];
			new_tri.vertices[1] = model_transformed[indices[i + 1]];
			new_tri.vertices[2] = model_transformed[indices[i + 2]];
			triangles.push_back(new_tri);
		}
	}
}

namespace components {
	void particle_group::draw(resources::buffer& triangles, const components::transform::state& transform, vec2<> camera_pos) {
		for (auto& it : particles) {
			auto temp_alpha = it.face.color.a;

			if (it.should_disappear)
				it.face.color.a = ((it.max_lifetime_ms - it.lifetime_ms) / it.max_lifetime_ms) * temp_alpha;

			it.face.draw(triangles, components::transform::state(it.pos, it.rotation), camera_pos);
			it.face.color.a = temp_alpha;
		}
	}

	bool particle_group::is_visible(rects::xywh visibility_aabb, const components::transform::state& transform) {
		/* will be visible most of the time */
		return true;
	}
}
