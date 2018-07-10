#include "augs/misc/add_concave_polygon.h"

#include "augs/drawing/polygon_draw.h"
#include "augs/math/math.h"
#include "augs/drawing/drawing.h"
#include "augs/drawing/grid_render_settings.h"
#include "augs/texture_atlas/atlas_entry.h"
#include "augs/drawing/polygon.h"
#include "augs/drawing/general_border.h"
#include "augs/misc/constant_size_vector.h"

namespace augs {

	const drawer& drawer::color_overlay(
		const atlas_entry tex,
		const vec2i screen_size,
		const rgba color
	) const {
		return aabb(tex, { { 0, 0 }, { screen_size } }, color);
	}

	const drawer& drawer::aabb(
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

	const drawer& drawer::aabb_lt(
		const atlas_entry tex,
		const vec2 left_top,
		const rgba color
	) const {
		return aabb(tex, ltrb(left_top, vec2(tex.get_original_size())), color);
	}

	const drawer& drawer::aabb_centered(
		const atlas_entry tex,
		const vec2 center,
		const vec2i size,
		const rgba color
	) const {
		return aabb(tex, ltrb(center - (size / 2), size), color);
	}

	const drawer& drawer::aabb_centered(
		const atlas_entry tex,
		const vec2 center,
		const rgba color
	) const {
		return aabb_centered(tex, center, tex.get_original_size(), color);
	}

	const drawer& drawer::aabb_lt_clipped(
		const atlas_entry tex,
		const vec2 left_top,
		ltrb clipper,
		const rgba color,
		const flip_flags flip
	) const {
		return aabb_clipped(tex, { left_top, vec2(tex.get_original_size()) }, clipper, color, flip);
	}

	const drawer& drawer::aabb_clipped(
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


	const drawer& drawer::border(
		const atlas_entry tex,
		ltrb bordered,
		const rgba color,
		border_input in
	) const {
		augs::general_border(
			bordered.round_fract(),
			in.get_total_expansion(),
			in.width,
			[this, tex, color](const ltrb line) {
				aabb(tex, line, color);
			}
		);

		return *this;
	}

	const drawer& drawer::aabb_with_border(
		const atlas_entry tex,
		const ltrb origin,
		const rgba inside_color,
		const rgba border_color,
		const border_input in
	) const {
		aabb(tex, origin, inside_color);
		border(tex, origin, border_color, in);

		return *this;
	}

	const drawer& drawer::rectangular_clock(
		const atlas_entry tex,
		const ltrb origin,
		const rgba color,
		const float ratio
	) const {
		if (ratio > 0.f) {
			if (ratio > 1.f) {
				aabb(tex, origin, color);
			}
			else {
				const auto twelve_o_clock = (origin.right_top() + origin.left_top()) / 2;

				augs::constant_size_vector<vec2, 7> verts;
				verts.push_back(origin.get_center());

				const auto intersection = rectangle_ray_intersection(
					origin.get_center() + vec2::from_degrees(-90 + 360 * (1 - ratio)) * (origin.w() + origin.h()),
					origin.get_center(),
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

				augs::polygon<20, 20> poly;

				augs::add_concave_polygon(poly, { concave.begin(), concave.end() });

				map_uv(poly.vertices, uv_mapping_mode::STRETCH);

				augs::draw(poly, *this, tex, {});
			}
		}

		return *this;
	}

	const drawer& drawer::line(
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
	
	const drawer& drawer::grid(
		const atlas_entry tex,
		const vec2i screen_size,
		const unsigned minimum_unit,
		const camera_eye cone,
		const grid_render_settings& settings
	) const {
		const auto cols = [&settings]() {
			auto result = settings.line_colors;

			for (auto& c : result) {
				c.multiply_alpha(settings.alpha_multiplier);
			}

			return result;
		}();

		const auto origin = screen_size / 2;
		const auto maximum_unit = static_cast<unsigned>(1 << settings.maximum_power_of_two);

		for (unsigned unit = maximum_unit, color_index = 0;
		   	unit >= minimum_unit;
			unit /= 2, ++color_index
		) {
			const auto screen_pixels_distance = unit * cone.zoom;

			if (screen_pixels_distance < settings.hide_grids_smaller_than) {
				continue;
			}

			const auto offset = -1 * (cone.transform.pos * cone.zoom);

			auto skip = [&](const int i){
				if (unit < maximum_unit) {
					const auto logical_coordinate = i * unit;

					if (logical_coordinate % (unit * 2) == 0) {
						return true;
					}
				}

				return false;
			};

			{
				const auto closest_i = static_cast<int>(offset.x / screen_pixels_distance);

				for (int i = closest_i + 1;; ++i) {
					if (skip(i)) {
						continue;
					}

					const auto x = offset.x + origin.x + screen_pixels_distance * i;

					if (x > screen_size.x) {
						break;
					}

					const auto top = vec2i(x, 0);
					const auto bottom = vec2i(x, screen_size.y);

					line(tex, top, bottom, 1, cols[color_index % cols.size()], {});
				}

				for (int i = closest_i;; --i) {
					if (skip(i)) {
						continue;
					}

					const auto x = offset.x + origin.x + screen_pixels_distance * i;

					if (x < 0) {
						break;
					}

					const auto top = vec2i(x, 0);
					const auto bottom = vec2i(x, screen_size.y);

					line(tex, top, bottom, 1, cols[color_index % cols.size()], {});
				}
			}

			{
				const auto closest_i = static_cast<int>(offset.y / screen_pixels_distance);

				for (int i = closest_i + 1;; ++i) {
					if (skip(i)) {
						continue;
					}

					const auto y = offset.y + origin.y + screen_pixels_distance * i;

					if (y > screen_size.y) {
						break;
					}

					const auto top = vec2i(0, y);
					const auto bottom = vec2i(screen_size.x, y);

					line(tex, top, bottom, 1, cols[color_index % cols.size()], {});
				}

				for (int i = closest_i;; --i) {
					if (skip(i)) {
						continue;
					}

					const auto y = offset.y + origin.y + screen_pixels_distance * i;

					if (y < 0) {
						break;
					}

					const auto top = vec2i(0, y);
					const auto bottom = vec2i(screen_size.x, y);

					line(tex, top, bottom, 1, cols[color_index % cols.size()], {});
				}
			}
		}

		return *this;
	}

	const line_drawer& line_drawer::line(
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

	const line_drawer& line_drawer::dashed_line(
		const atlas_entry tex,
		const vec2 from,
		const vec2 to,
		const rgba color,
		const float dash_length,
		const float dash_velocity,
		const double global_time_seconds
	) const {
		float dash_end = static_cast<float>(fmod(global_time_seconds*dash_velocity, dash_length * 2));
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