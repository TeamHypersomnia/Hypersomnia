#pragma once
#include "augs/drawing/drawing.h"
#include "augs/drawing/make_sprite_points.h"
#include "augs/drawing/make_sprite.h"

namespace augs {
	FORCE_INLINE const drawer& drawer::color_overlay(
		const atlas_entry tex,
		const vec2i screen_size,
		const rgba color,
		const flip_flags flip
	) const {
		return aabb(tex, { { 0, 0 }, { screen_size } }, color, flip);
	}

	FORCE_INLINE const drawer& drawer::aabb_bordered(
		const atlas_entry entry,
		const ltrb origin,
		const rgba inside_color,
		const rgba border_color
	) const {
		const vec2i offsets[4] = {
			vec2i(-1, 0), vec2i(1, 0), vec2i(0, 1), vec2i(0, -1)
		};

		for (const auto& o : offsets) {
			const auto offset_origin = ltrb(origin) += o;

			aabb(entry, offset_origin, border_color);
		}

		aabb(entry, origin, inside_color);
		return *this;
	}

	FORCE_INLINE const drawer& drawer::aabb(
		const atlas_entry tex,
		const ltrb origin,
		const rgba color,
		const flip_flags flip
	) const {
		auto p = std::array<augs::vertex, 4>();

		p[0].pos.x = p[3].pos.x = origin.l;
		p[0].pos.y = p[1].pos.y = origin.t;
		p[1].pos.x = p[2].pos.x = origin.r;
		p[2].pos.y = p[3].pos.y = origin.b;

		auto p1 = vec2(0.f, 0.f);
		auto p2 = vec2(1.f, 1.f);

		if (flip.horizontally) {
			p1.x = 1.f - p1.x;
			p2.x = 1.f - p2.x;
		}

		if (flip.vertically) {
			p1.y = 1.f - p1.y;
			p2.y = 1.f - p2.y;
		}

		p[0].color = p[1].color = p[2].color = p[3].color = color;

		p[0].texcoord = tex.get_atlas_space_uv({ p1.x, p1.y });
		p[1].texcoord = tex.get_atlas_space_uv({ p2.x, p1.y });
		p[2].texcoord = tex.get_atlas_space_uv({ p2.x, p2.y });
		p[3].texcoord = tex.get_atlas_space_uv({ p1.x, p2.y });

		{
			auto out = augs::vertex_triangle();

			out.vertices[0] = p[0];
			out.vertices[1] = p[1];
			out.vertices[2] = p[2];

			output_buffer.push_back(out);
		}

		{
			auto out = augs::vertex_triangle();

			out.vertices[0] = p[2];
			out.vertices[1] = p[3];
			out.vertices[2] = p[0];

			output_buffer.push_back(out);
		}

		return *this;
	}

	FORCE_INLINE const drawer& drawer::aabb_lt(
		const atlas_entry tex,
		const vec2 left_top,
		const rgba color
	) const {
		return aabb(tex, ltrb(left_top, vec2(tex.get_original_size())), color);
	}

	FORCE_INLINE const drawer& drawer::aabb_centered(
		const atlas_entry tex,
		const vec2 center,
		const vec2i size,
		const rgba color
	) const {
		return aabb(tex, ltrb(center - (size / 2), size), color);
	}

	FORCE_INLINE const drawer& drawer::aabb_centered(
		const atlas_entry tex,
		const vec2 center,
		const rgba color
	) const {
		return aabb_centered(tex, center, tex.get_original_size(), color);
	}

	FORCE_INLINE const drawer& drawer::aabb_lt_clipped(
		const atlas_entry tex,
		const vec2 left_top,
		ltrb clipper,
		const rgba color,
		const flip_flags flip
	) const {
		return aabb_clipped(tex, { left_top, vec2(tex.get_original_size()) }, clipper, color, flip);
	}

	FORCE_INLINE const drawer& drawer::aabb_clipped(
		const atlas_entry tex,
		const ltrb origin,
		ltrb clipper,
		const rgba color,
		const flip_flags flip
	) const {
		ltrb rc = origin;

		if (!rc.good()) {
			return *this;
		}

		if (clipper.good() && !rc.clip_by(clipper)) {
			return *this;
		}

		auto p = std::array<augs::vertex, 4>();

		float tw = 1.f / origin.w();
		float th = 1.f / origin.h();

		p[0].color = p[1].color = p[2].color = p[3].color = color;

		auto p1 = vec2(
			((p[0].pos.x = p[3].pos.x = rc.l) - origin.l) * tw,
			((p[0].pos.y = p[1].pos.y = rc.t) - origin.t) * th
		);

		auto p2 = vec2(
			((p[1].pos.x = p[2].pos.x = rc.r) - origin.r) * tw + 1.0f,
			((p[2].pos.y = p[3].pos.y = rc.b) - origin.b) * th + 1.0f
		);

		if (flip.horizontally) {
			p1.x = 1.f - p1.x;
			p2.x = 1.f - p2.x;
		}

		if (flip.vertically) {
			p1.y = 1.f - p1.y;
			p2.y = 1.f - p2.y;
		}

		p[0].texcoord = tex.get_atlas_space_uv({ p1.x, p1.y });
		p[1].texcoord = tex.get_atlas_space_uv({ p2.x, p1.y });
		p[2].texcoord = tex.get_atlas_space_uv({ p2.x, p2.y });
		p[3].texcoord = tex.get_atlas_space_uv({ p1.x, p2.y });

		{
			auto out = augs::vertex_triangle();

			out.vertices[0] = p[0];
			out.vertices[1] = p[1];
			out.vertices[2] = p[2];

			output_buffer.push_back(out);
		}

		{
			auto out = augs::vertex_triangle();

			out.vertices[0] = p[2];
			out.vertices[1] = p[3];
			out.vertices[2] = p[0];

			output_buffer.push_back(out);
		}

		return *this;
	}

	FORCE_INLINE const drawer& drawer::line(
		const atlas_entry tex,
		const vec2 from,
		const vec2 to,
		const float line_width,
		const rgba color,
		const flip_flags flip
	) const {
		const auto line_pos = (from + to) / 2;
		const auto line_size = vec2((from - to).length(), line_width);
		const auto line_angle = (to - from).degrees();
		
		const auto line_points = make_sprite_points(line_pos, line_size, line_angle);
		const auto tris = make_sprite_triangles(tex, line_points, color, flip);

		output_buffer.push_back(tris[0]);
		output_buffer.push_back(tris[1]);

		return *this;
	}
	
	FORCE_INLINE const line_drawer& line_drawer::line(
		const atlas_entry tex, 
		const vec2 from, 
		const vec2 to, 
		const rgba color
	) const {
		const auto line_pos = (from + to) / 2;
		const auto line_size = vec2((from - to).length(), 0);
		const auto line_angle = (to - from).degrees();

		const auto line_points = make_sprite_points(line_pos, line_size, line_angle);
		const auto tris = make_sprite_triangles(tex, line_points, color);

		output_buffer.push_back({ tris[0].vertices[0], tris[0].vertices[1] });

		return *this;
	}

	FORCE_INLINE const line_drawer& line_drawer::dashed_line(
		const atlas_entry tex,
		const vec2 from,
		const vec2 to,
		const rgba color,
		const float dash_length,
		const float dash_velocity,
		const double global_time_seconds
	) const {
		float dash_end = static_cast<float>(std::fmod(global_time_seconds*dash_velocity, dash_length * 2));
		float dash_begin = dash_end - dash_length;
		dash_begin = std::max(dash_begin, 0.f);

		auto line_vector = to - from;
		const auto line_length = line_vector.length();
		line_vector /= line_length;

		while (dash_begin < line_length) {
			line(
				tex,
				from + line_vector * dash_begin,
				from + line_vector * dash_end,
				color
			);

			dash_begin = dash_end + dash_length;
			dash_end = dash_begin + dash_length;
			dash_end = std::min(dash_end, line_length);
		}

		return *this;
	}
}

