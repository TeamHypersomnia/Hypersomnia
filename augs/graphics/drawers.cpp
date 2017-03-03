#include "augs/graphics/drawers.h"

#include "game/assets/texture_id.h"
#include "game/resources/manager.h"

#include "augs/gui/stroke.h"

#include "game/components/sprite_component.h"
#include "game/components/polygon_component.h"

namespace augs {
	ltrb draw_clipped_rect(vertex_triangle_buffer& v, const ltrb origin, const augs::texture_atlas_entry& tex, const rgba color, const ltrb clipper, const bool flip_horizontally) {
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

		p[0].color = p[1].color = p[2].color = p[3].color = color;

		auto p1 = vec2(((p[0].pos.x = p[3].pos.x = rc.l) - origin.l) * tw,
			((p[0].pos.y = p[1].pos.y = rc.t) - origin.t) * th);

		auto p2 = vec2(((p[1].pos.x = p[2].pos.x = rc.r) - origin.r) * tw + 1.0f,
			((p[2].pos.y = p[3].pos.y = rc.b) - origin.b) * th + 1.0f);

		if (flip_horizontally) {
			p1.x = 1.f - p1.x;
			p2.x = 1.f - p2.x;
		}

		p[0].texcoord = tex.get_atlas_space_uv({p1.x, p1.y});
		p[1].texcoord = tex.get_atlas_space_uv({p2.x, p1.y});
		p[2].texcoord = tex.get_atlas_space_uv({p2.x, p2.y});
		p[3].texcoord = tex.get_atlas_space_uv({p1.x, p2.y});

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
	
	void draw_rect_with_border(vertex_triangle_buffer& v, const ltrb origin, const rgba inside_color, const rgba border_color, const int border_spacing) {
		draw_rect_with_border(v, origin, assets::texture_id::BLANK, inside_color, border_color, border_spacing);
	}

	void draw_rect_with_border(vertex_triangle_buffer& v, const ltrb origin, const assets::texture_id tex, const rgba inside_color, const rgba border_color, const int border_spacing) {
		draw_rect(v, origin, tex, inside_color);

		augs::gui::solid_stroke stroke;
		stroke.set_material(augs::gui::material( tex, border_color ));
		stroke.set_width(1);
		stroke.draw(v, origin, ltrb(), border_spacing);
	}

	void draw_rect(vertex_triangle_buffer& v, const ltrb origin, const rgba color) {
		draw_rect(v, origin, assets::texture_id::BLANK, color);
	}

	void draw_rect(vertex_triangle_buffer& v, const ltrb origin, const assets::texture_id id, const rgba color) {
		draw_rect(v, origin, *id, color);
	}

	void draw_rect(vertex_triangle_buffer& v, const vec2 origin, const assets::texture_id id, const rgba color) {
		draw_rect(v, xywh {origin.x, origin.y, static_cast<float>(assets::get_size(id).x), static_cast<float>(assets::get_size(id).y) }, *id, color);
	}

	void draw_rect(vertex_triangle_buffer& v, const ltrb origin, const texture_atlas_entry& tex, const rgba color) {
		augs::vertex p[4];

		p[0].pos.x = p[3].pos.x = origin.l;
		p[0].pos.y = p[1].pos.y = origin.t;
		p[1].pos.x = p[2].pos.x = origin.r;
		p[2].pos.y = p[3].pos.y = origin.b;

		vec2 p1(0.f, 0.f);
		vec2 p2(1.f, 1.f);

		p[0].color = p[1].color = p[2].color = p[3].color = color;


		p[0].texcoord = tex.get_atlas_space_uv({ p1.x, p1.y });
		p[1].texcoord = tex.get_atlas_space_uv({ p2.x, p1.y });
		p[2].texcoord = tex.get_atlas_space_uv({ p2.x, p2.y });
		p[3].texcoord = tex.get_atlas_space_uv({ p1.x, p2.y });

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
		const augs::texture_atlas_entry& considered_texture,
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
			t1.vertices[i].texcoord = considered_texture.get_atlas_space_uv(t1.vertices[i].texcoord);
			t2.vertices[i].texcoord = considered_texture.get_atlas_space_uv(t2.vertices[i].texcoord);
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

	void draw_line(vertex_triangle_buffer& v, const vec2 from, const vec2 to, const float line_width, const texture_atlas_entry& tex, const rgba color, const bool flip_horizontally) {
		const auto points = make_sprite_points((from + to)/2, vec2((from - to).length(), line_width), (to - from).degrees());
		const auto tris = make_sprite_triangles(points, tex, color, flip_horizontally);

		v.push_back(tris[0]);
		v.push_back(tris[1]);
	}

	void draw_line(vertex_line_buffer& v, const vec2 from, const vec2 to, const texture_atlas_entry& tex, const rgba color) {
		const auto points = make_sprite_points((from + to) / 2, vec2((from - to).length(), 0), (to - from).degrees());
		const auto tris = make_sprite_triangles(points, tex, color);

		v.push_back({ tris[0].vertices[0], tris[0].vertices[1] });
	}

	void draw_dashed_line(vertex_line_buffer& v, const vec2 from, const vec2 to, const texture_atlas_entry& tex, const rgba color, const float dash_length, const float dash_velocity, const float global_time_seconds) {
		auto dash_end = fmod(global_time_seconds*dash_velocity, dash_length * 2);
		float dash_begin = dash_end - dash_length;
		dash_begin = std::max(dash_begin, 0.f);

		auto line_vector = to - from;
		const auto line_length = line_vector.length();
		line_vector /= line_length;

		while (dash_begin < line_length) {
			augs::draw_line(
				v,
				from + line_vector * dash_begin,
				from + line_vector * dash_end,
				tex,
				color
			);

			dash_begin = dash_end + dash_length;
			dash_end = dash_begin + dash_length;
			dash_end = std::min(dash_end, line_length);
		}
	}

	void draw_rectangle_clock(
		vertex_triangle_buffer& v,
		const float ratio,
		const ltrb origin,
		const rgba color
	) {
		if (ratio > 0.f) {
			if (ratio > 1.f) {
				draw_rect(v, origin, color);
			}
			else {
				const auto twelve_o_clock = (origin.right_top() + origin.left_top()) / 2;
				
				augs::constant_size_vector<vec2, 7> verts;
				verts.push_back(origin.center());

				const auto intersection = rectangle_ray_intersection(
					origin.center() + vec2().set_from_degrees(-90 + 360 * (1 - ratio)) * (origin.w() + origin.h()),
					origin.center(),
					origin
				);

				ensure(intersection.hit);

				verts.push_back(intersection.intersection);

				if (ratio > 0.875f) {
					verts.push_back(origin.right_top());
					verts.push_back(origin.right_bottom());
					verts.push_back(origin.left_bottom());
					verts.push_back(origin.left_top());
					verts.push_back(twelve_o_clock);
				}
				else if (ratio > 0.625f) {
					verts.push_back(origin.right_bottom());
					verts.push_back(origin.left_bottom());
					verts.push_back(origin.left_top());
					verts.push_back(twelve_o_clock);
				}
				else if (ratio > 0.375f) {
					verts.push_back(origin.left_bottom());
					verts.push_back(origin.left_top());
					verts.push_back(twelve_o_clock);
				}
				else if (ratio > 0.125f) {
					verts.push_back(origin.left_top());
					verts.push_back(twelve_o_clock);
				}
				else {
					verts.push_back(twelve_o_clock);
				}

				augs::constant_size_vector<augs::vertex, 7> concave;

				for (const auto& v : verts) {
					augs::vertex vv;
					vv.color = color;
					vv.pos = v;
					concave.push_back(vv);
				}

				components::polygon poly;
				poly.add_concave_polygon({ concave.begin(), concave.end() } );
				
				poly.automatically_map_uv(
					assets::texture_id::BLANK, 
					components::polygon::uv_mapping_mode::STRETCH
				);

				components::polygon::drawing_input in(v);
				poly.draw(in);
			}
		}
	}
}