#include "augs/misc/from_concave_polygon.h"

#include "augs/drawing/polygon.hpp"
#include "augs/math/math.h"
#include "augs/drawing/drawing.h"
#include "augs/drawing/grid_render_settings.h"
#include "augs/texture_atlas/atlas_entry.h"
#include "augs/drawing/polygon.h"
#include "augs/drawing/general_border.h"
#include "augs/misc/constant_size_vector.h"
#include "augs/drawing/make_sprite.h"
#include "augs/drawing/drawing.hpp"

namespace augs {
	const drawer& drawer::border(
		const atlas_entry tex,
		ltrb bordered,
		const rgba color,
		const border_input in
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

	const drawer& drawer::border(
		const atlas_entry tex,
		const vec2 size,
		const vec2 pos,
		const float rotation,
		const rgba color,
		const border_input in
	) const {
		auto bordered = ltrb::center_and_size(pos, size);

		augs::general_border(
			bordered.round_fract(),
			in.get_total_expansion(),
			in.width,
			[&](const ltrb line) {
				auto verts = line.get_vertices();

				for (auto& v : verts) {
					v.rotate(rotation, pos);
				}

				const auto triangles = make_sprite_triangles(tex, verts, color);

				output_buffer.push_back(triangles[0]);
				output_buffer.push_back(triangles[1]);
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

				std::reverse(verts.begin(), verts.end());
				augs::constant_size_vector<augs::vertex, 7> concave;

				for (const auto& v : verts) {
					augs::vertex vv;
					vv.color = color;
					vv.pos = v;
					concave.push_back(vv);
				}

				augs::polygon<20, 20> poly;

				augs::from_concave_polygon(poly, { concave.begin(), concave.end() });

				map_uv(poly.vertices, uv_mapping_mode::STRETCH);

				augs::draw(poly, *this, tex, {});
			}
		}

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
				c.mult_alpha(settings.alpha_multiplier);
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

	const line_drawer& line_drawer::dashed_circular_sector(
		const atlas_entry tex,
		const vec2 p,
		const float r,
		const rgba color,
		const float rot,
		const float a,
		const float dash_len
	) const {
		auto draw_dash = [&](const auto from, const auto to) {
			dashed_line(
				tex,
				from, 
				to,
				color,
				dash_len,
				dash_len,
				0.f
			);
		};

		const auto left_angle = rot - a / 2;
		const auto right_angle = rot + a / 2;

		draw_dash(
			p, 
			p + vec2::from_degrees(left_angle) * r
		);

		draw_dash(
			p, 
			p + vec2::from_degrees(right_angle) * r
		);

		const auto perim = 2 * PI<float> * r;
		const auto fraction = dash_len / perim;
		const auto df = 360.f * fraction;

		for (float l = 0.f; l <= a; l += df * 2) {
			const auto p1 = p + vec2::from_degrees(left_angle + l) * r;

			const auto next_angle = std::min(right_angle, left_angle + l + df);
			const auto p2 = p + vec2::from_degrees(next_angle) * r;

			line(tex, p1, p2, color);
		}
		return *this;
	}

	const line_drawer& line_drawer::border(
		const atlas_entry tex,
		const vec2 size,
		const vec2 pos,
		const float rotation,
		const rgba color,
		const border_input in
	) const {
		auto bordered = ltrb::center_and_size(pos, size);

		augs::general_border(
			bordered.round_fract(),
			in.get_total_expansion(),
			in.width,
			[&](const ltrb line) {
				auto verts = line.get_vertices();

				for (auto& v : verts) {
					v.rotate(rotation, pos);
				}

				const auto tris = make_sprite_triangles(tex, verts, color);

				output_buffer.push_back({ tris[0].vertices[0], tris[0].vertices[1] });
			}
		);

		return *this;
	}

	const line_drawer& line_drawer::border_dashed(
		const atlas_entry tex,
		const vec2 size,
		const vec2 pos,
		const float rotation,
		const rgba color,

		float dash_length,
		float dash_velocity,
		double global_time_seconds,

		const border_input in
	) const {
		auto bordered = ltrb::center_and_size(pos, size);

		augs::general_border_from_to(
			bordered.round_fract(),
			in.get_total_expansion(),
			[&](vec2 from, vec2 to) {
				from.rotate(rotation, pos);
				to.rotate(rotation, pos);
				dashed_line(tex, from, to, color, dash_length, dash_velocity, global_time_seconds);
			}
		);

		return *this;
	}
}