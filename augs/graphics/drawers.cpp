#include "augs/graphics/drawers.h"
#include "augs/texture_baker/texture_baker.h"

#include "game/assets/texture_id.h"
#include "game/resources/manager.h"

#include "augs/gui/stroke.h"

#include "game/components/sprite_component.h"

namespace augs {
	ltrb draw_clipped_rect(vertex_triangle_buffer& v, const ltrb origin, const augs::texture& tex, const rgba color, const ltrb clipper, const bool flip_horizontally) {
		ltrb rc = origin;

		if (!rc.good()) {
			return rc;
		}

		if (clipper.good() && !rc.clip_by(clipper)) {
			return rc;
		}

		augs::vertex p[4];

		float tw = 1.f / origin.w();
		float th = 1.f / origin.h();

		rects::texture<float> diff(((p[0].pos.x = p[3].pos.x = rc.l) - origin.l) * tw,
			((p[0].pos.y = p[1].pos.y = rc.t) - origin.t) * th,
			((p[1].pos.x = p[2].pos.x = rc.r) - origin.r) * tw + 1.0f,
			((p[2].pos.y = p[3].pos.y = rc.b) - origin.b) * th + 1.0f);

		p[0].color = p[1].color = p[2].color = p[3].color = color;

		if (flip_horizontally) {
			diff.u1 = 1.f - diff.u1;
			diff.u2 = 1.f - diff.u2;
		}

		tex.get_uv(diff.u1, diff.v1, p[0].texcoord.x, p[0].texcoord.y);
		tex.get_uv(diff.u2, diff.v1, p[1].texcoord.x, p[1].texcoord.y);
		tex.get_uv(diff.u2, diff.v2, p[2].texcoord.x, p[2].texcoord.y);
		tex.get_uv(diff.u1, diff.v2, p[3].texcoord.x, p[3].texcoord.y);

		augs::vertex_triangle out[2];
		out[0].vertices[0] = p[0];
		out[0].vertices[1] = p[1];
		out[0].vertices[2] = p[2];

		out[1].vertices[0] = p[2];
		out[1].vertices[1] = p[3];
		out[1].vertices[2] = p[0];

		v.push_back(out[0]);
		v.push_back(out[1]);

		return rc;
	}
	
	void draw_rect_with_border(vertex_triangle_buffer& v, const ltrb origin, const rgba inside_color, const rgba border_color) {
		draw_rect_with_border(v, origin, assets::texture_id::BLANK, inside_color, border_color);
	}

	void draw_rect_with_border(vertex_triangle_buffer& v, const ltrb origin, const assets::texture_id tex, const rgba inside_color, const rgba border_color) {
		draw_rect(v, origin, tex, inside_color);

		augs::gui::solid_stroke stroke;
		stroke.set_material(augs::gui::material( tex, border_color ));
		stroke.set_width(1);
		stroke.draw(v, origin);
	}

	void draw_rect(vertex_triangle_buffer& v, const ltrb origin, const rgba color) {
		draw_rect(v, origin, assets::texture_id::BLANK, color);
	}

	void draw_rect(vertex_triangle_buffer& v, const ltrb origin, const assets::texture_id id, const rgba color) {
		draw_rect(v, origin, *id, color);
	}

	void draw_rect(vertex_triangle_buffer& v, const vec2 origin, const assets::texture_id id, const rgba color) {
		draw_rect(v, xywh {origin.x, origin.y, static_cast<float>((*id).get_size().x), static_cast<float>((*id).get_size().y) }, *id, color);
	}

	void draw_rect(vertex_triangle_buffer& v, const ltrb origin, const texture& tex, const rgba color) {
		augs::vertex p[4];

		p[0].pos.x = p[3].pos.x = origin.l;
		p[0].pos.y = p[1].pos.y = origin.t;
		p[1].pos.x = p[2].pos.x = origin.r;
		p[2].pos.y = p[3].pos.y = origin.b;

		rects::texture<float> diff(0.f, 0.f, 1.f, 1.f);

		p[0].color = p[1].color = p[2].color = p[3].color = color;

		tex.get_uv(diff.u1, diff.v1, p[0].texcoord.x, p[0].texcoord.y);
		tex.get_uv(diff.u2, diff.v1, p[1].texcoord.x, p[1].texcoord.y);
		tex.get_uv(diff.u2, diff.v2, p[2].texcoord.x, p[2].texcoord.y);
		tex.get_uv(diff.u1, diff.v2, p[3].texcoord.x, p[3].texcoord.y);

		augs::vertex_triangle out[2];

		out[0].vertices[0] = p[0];
		out[0].vertices[1] = p[1];
		out[0].vertices[2] = p[2];

		out[1].vertices[0] = p[2];
		out[1].vertices[1] = p[3];
		out[1].vertices[2] = p[0];

		v.push_back(out[0]);
		v.push_back(out[1]);
	}
	
	std::array<vec2, 4> make_sprite_points(const vec2 pos, const vec2 size, const float rotation_degrees, const renderable_positioning_type positioning) {
		std::array<vec2, 4> v;

		if (positioning == renderable_positioning_type::CENTER) {
			const vec2 origin = pos;
			const vec2 half_size = size / 2.f;

			v[0] = pos - half_size;
			v[1] = pos + vec2(size.x, 0.f) - half_size;
			v[2] = pos + size - half_size;
			v[3] = pos + vec2(0.f, size.y) - half_size;

			v[0].rotate(rotation_degrees, origin);
			v[1].rotate(rotation_degrees, origin);
			v[2].rotate(rotation_degrees, origin);
			v[3].rotate(rotation_degrees, origin);
		}
		else {
			const vec2 origin = pos + size / 2.f;

			v[0] = pos;
			v[1] = pos + vec2(size.x, 0.f);
			v[2] = pos + size;
			v[3] = pos + vec2(0.f, size.y);

			v[0].rotate(rotation_degrees, origin);
			v[1].rotate(rotation_degrees, origin);
			v[2].rotate(rotation_degrees, origin);
			v[3].rotate(rotation_degrees, origin);
		}

		return v;
	}

	std::array<vertex_triangle, 2> make_sprite_triangles(
		const std::array<vec2, 4> v,
		const augs::texture& considered_texture,
		const rgba col,
		const bool flip_horizontally,
		const bool flip_vertically) {

		std::array<vertex_triangle, 2> tris;

		vertex_triangle& t1 = tris[0];
		vertex_triangle& t2 = tris[1];

		vec2 texcoords[] = {
			vec2(0.f, 0.f),
			vec2(1.f, 0.f),
			vec2(1.f, 1.f),
			vec2(0.f, 1.f)
		};

		if (flip_horizontally) {
			for (auto& v : texcoords) {
				v.x = 1.f - v.x;
			}
		}
		if (flip_vertically) {
			for (auto& v : texcoords) {
				v.y = 1.f - v.y;
			}
		}

		t1.vertices[0].texcoord = t2.vertices[0].texcoord = texcoords[0];
		t2.vertices[1].texcoord = texcoords[1];
		t1.vertices[1].texcoord = t2.vertices[2].texcoord = texcoords[2];
		t1.vertices[2].texcoord = texcoords[3];

		for (int i = 0; i < 3; ++i) {
			considered_texture.get_uv(t1.vertices[i].texcoord);
			considered_texture.get_uv(t2.vertices[i].texcoord);
		}

		t1.vertices[0].pos = t2.vertices[0].pos = static_cast<vec2i>(v[0]);
		t2.vertices[1].pos = static_cast<vec2i>(v[1]);
		t1.vertices[1].pos = t2.vertices[2].pos = static_cast<vec2i>(v[2]);
		t1.vertices[2].pos = static_cast<vec2i>(v[3]);

		t1.vertices[0].color = t2.vertices[0].color = col;
		t1.vertices[1].color = t2.vertices[1].color = col;
		t1.vertices[2].color = t2.vertices[2].color = col;

		return tris;
	}

	void draw_line(vertex_triangle_buffer& v, const vec2 from, const vec2 to, const float line_width, const texture& tex, const rgba color, const bool flip_horizontally) {
		const auto points = make_sprite_points((from + to)/2, vec2((from - to).length(), line_width), (to - from).degrees());
		const auto tris = make_sprite_triangles(points, tex, color, flip_horizontally);

		v.push_back(tris[0]);
		v.push_back(tris[1]);
	}

	void draw_line(vertex_line_buffer& v, const vec2 from, const vec2 to, const texture& tex, const rgba color) {
		const auto points = make_sprite_points((from + to) / 2, vec2((from - to).length(), 0), (to - from).degrees());
		const auto tris = make_sprite_triangles(points, tex, color);

		v.push_back({ tris[0].vertices[0], tris[0].vertices[1] });
	}
}