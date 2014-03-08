#include "stdafx.h"

#include "render_info.h"

#include <algorithm>
#include "entity_system/entity.h"

#include "../components/physics_component.h"
#include "../components/particle_group_component.h"

#include "misc/sorted_vector.h"

#include "3rdparty/polypartition/polypartition.h"

std::vector<render_system::debug_line> global_debug;

namespace resources {
	void set_polygon_color(renderable* poly, graphics::pixel_32 col) {
		polygon* p = (polygon*) poly;

		for (auto& v : p->model) {
			v.color = col;
		}
	}

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

	std::vector<vec2<>> renderable::get_vertices() {
		return std::vector<vec2<>>();
	}

	sprite::sprite(texture_baker::texture* tex, graphics::pixel_32 color) : tex(tex), color(color), rotation_offset(0.f) {
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

	void sprite::draw(draw_input in) {
		auto& triangles = *in.output;

		if (tex == nullptr) return;
		vec2<> v[4];

		auto center = vec2<>(in.visible_area.w(), in.visible_area.h()) / 2;

		auto target_position = in.transform.pos - in.camera_transform.pos + center;
		make_rect(target_position, vec2<>(size), in.transform.rotation + rotation_offset, v);

		/* rotate around the center of the screen */
		for (auto& vert : v)
			vert.rotate(in.camera_transform.rotation, center);

		vertex_triangle t1, t2;
		t1.vertices[0].color = t2.vertices[0].color = color;
		t1.vertices[1].color = t2.vertices[1].color = color;
		t1.vertices[2].color = t2.vertices[2].color = color;

		vec2<> texcoords[] = {
			vec2<>(0.f, 0.f),
			vec2<>(1.f, 0.f),
			vec2<>(1.f, 1.f),
			vec2<>(0.f, 1.f)
		};

		if (in.additional_info) {
			if (in.additional_info->flip_horizontally)
				for (auto& v : texcoords)
					v.x = 1.f - v.x; 
				if (in.additional_info->flip_vertically)
				for (auto& v : texcoords)
					v.y = 1.f - v.y;
		}

		t1.vertices[0].texcoord = t2.vertices[0].texcoord = texcoords[0];
		t2.vertices[1].texcoord =							texcoords[1];
		t1.vertices[1].texcoord = t2.vertices[2].texcoord = texcoords[2];
		t1.vertices[2].texcoord =							texcoords[3];

		for (int i = 0; i < 3; ++i) {
			tex->get_uv(t1.vertices[i].texcoord);
			tex->get_uv(t2.vertices[i].texcoord);
		}

		t1.vertices[0].pos = t2.vertices[0].pos = vec2<int>(v[0]);
		t2.vertices[1].pos = vec2<int>(v[1]);
		t1.vertices[1].pos = t2.vertices[2].pos = vec2<int>(v[2]);
		t1.vertices[2].pos = vec2<int>(v[3]);

		triangles.emplace_back(t1);
		triangles.emplace_back(t2);
	}

	bool sprite::is_visible(rects::xywh visibility_aabb, const components::transform::state& transform) {
		vec2<> v[4];
		make_rect(transform.pos, vec2<>(size), transform.rotation, v);

		typedef const vec2<>& vc;

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

		return rects::ltrb::get_aabb(v).hover(visibility_aabb);
	}
	
	std::vector<vec2<>> sprite::get_vertices() {
		std::vector<vec2<>> out;
		out.push_back(size / -2.f);
		out.push_back(size / -2.f + vec2<>(size.x, 0.f));
		out.push_back(size / -2.f + size);
		out.push_back(size / -2.f + vec2<>(0.f, size.y));
		return std::move(out);
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

	void polygon::add_concave(const concave& original_polygon) {
		if (original_polygon.vertices.empty()) return;
		int i1, i2;

		auto polygon = original_polygon;

		float area = 0;
		auto& vs = polygon.vertices;

		for (i1 = 0; i1 < vs.size(); i1++) {
			i2 = i1 + 1;
			if (i2 == vs.size()) i2 = 0;
			area += vs[i1].pos.x * vs[i2].pos.y - vs[i1].pos.y * vs[i2].pos.x;
		}

		/* ensure proper winding */
		if (area > 0) std::reverse(polygon.vertices.begin(), polygon.vertices.end());
		

		TPPLPoly inpoly;
		list<TPPLPoly> out_tris;

		TPPLPoly subject_poly;
		inpoly.Init(polygon.vertices.size());
		inpoly.SetHole(false);
		
		model.reserve(model.size() + polygon.vertices.size());

		int offset = model.size();
		for (size_t i = 0; i < polygon.vertices.size(); ++i) {
			model.push_back(polygon.vertices[i]);
			original_model.push_back(polygon.vertices[i].pos);
		}

		for (size_t i = 0; i < polygon.vertices.size(); ++i) {
			vec2<> p(polygon.vertices[i].pos);
			inpoly[i].x = p.x;
			inpoly[i].y = -p.y;
		}

		TPPLPartition partition;
		partition.Triangulate_EC(&inpoly, &out_tris);

		for (auto& out : out_tris) {
			for (int i = 0; i < 3; ++i) {
				auto new_tri_point = out.GetPoint(i);

				for (int j = offset; j < polygon.vertices.size(); ++j) {
					if (polygon.vertices[j].pos.compare(vec2<>(new_tri_point.x, -new_tri_point.y), 1.f)) {
						indices.push_back(j);
						break;
					}
				}
			}
		}
	}

	std::vector<vec2<>> polygon::get_vertices() {
		std::vector<vec2<>> out;

		for (auto& v : model) 
			out.push_back(v.pos);
		
		return std::move(out);
	}

	//void polygon::add_convex(const std::vector<vertex>& model) {
	//	convex_models.push_back(model);
	//}

	void polygon::draw(draw_input in) {
		vertex_triangle new_tri;
		auto camera_pos = in.camera_transform.pos;

		auto model_transformed = model;
		for (auto& v : model_transformed) {
			auto center = vec2<>(in.visible_area.w(), in.visible_area.h()) / 2;
			v.pos.rotate(in.transform.rotation, vec2<>(0, 0));
			v.pos += in.transform.pos - camera_pos + center;

			/* rotate around the center of the screen */
			v.pos.rotate(in.camera_transform.rotation, center);
		}

		for (size_t i = 0; i < indices.size(); i += 3) {
			new_tri.vertices[0] = model_transformed[indices[i]];
			new_tri.vertices[1] = model_transformed[indices[i + 1]];
			new_tri.vertices[2] = model_transformed[indices[i + 2]];
			in.output->push_back(new_tri);
		}
	}
}

namespace components {
	void particle_group::draw(draw_input in) {
		for (auto& s : stream_slots)
			for (auto& it : s.particles.particles) {
				auto temp_alpha = it.face.color.a;

				if (it.should_disappear)
					it.face.color.a = static_cast<graphics::color>(((it.max_lifetime_ms - it.lifetime_ms) / it.max_lifetime_ms) * static_cast<float>(temp_alpha));

				in.transform = components::transform::state(it.pos, it.rotation);
				it.face.draw(in);
				it.face.color.a = temp_alpha;
			}
	}

	bool particle_group::is_visible(rects::xywh visibility_aabb, const components::transform::state& transform) {
		/* will be visible most of the time */
		return true;
	}
}
