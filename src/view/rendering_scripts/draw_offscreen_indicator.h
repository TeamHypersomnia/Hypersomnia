#pragma once
#include "augs/math/math.h"
#include "augs/drawing/drawing.hpp"

inline void draw_offscreen_indicator(
	const augs::drawer& output,
	const bool draw_if_offscreen,
	const bool draw_if_onscreen,
	const rgba col,
	const augs::atlas_entry& indicator_tex,
	const vec2 indicator_pos,
	const vec2i screen_size,
	const bool should_rotate_indicator_tex,
	const std::optional<augs::atlas_entry> next_tex,
	const augs::baked_font& gui_font,
	std::string primary_text,
	const std::string& secondary_text,
	const vec2 viewed_character_pos,
	const offscreen_reference_type reference_type
) {
	using namespace augs::gui::text;

	const auto bbox = indicator_tex.get_original_size();

	const auto screen_aabb = ltrb(vec2::zero, screen_size);

	auto raycasted_aabb = screen_aabb;
	raycasted_aabb.r -= bbox.x;
	raycasted_aabb.b -= bbox.y;

	{
		if (draw_if_onscreen && !draw_if_offscreen) {
			const auto indicator_rect = ltrb(indicator_pos, indicator_tex.get_original_size());

			if (screen_aabb.hover(indicator_rect)) {
				output.aabb_lt(indicator_tex, indicator_pos, col);
			}

			return;
		}
		
		const bool is_onscreen = raycasted_aabb.hover(indicator_pos);
		const bool is_offscreen = !is_onscreen;

		if (is_onscreen) {
			if (draw_if_onscreen) {
				output.aabb_lt(indicator_tex, indicator_pos, col);
			}

			return;
		}

		if (is_offscreen) {
			if (!draw_if_offscreen) {
				return;
			}
		}
	}

	const auto reference_pos = 
		reference_type == offscreen_reference_type::SCREEN_CENTER ?
		vec2(screen_size) / 2 :
		viewed_character_pos
	;

	const auto raycast_a = indicator_pos;
	const auto raycast_b = reference_pos;

	const auto edges = raycasted_aabb.make_edges();

	for (std::size_t e = 0; e < edges.size(); ++e) {
		const auto intersection = ::segment_segment_intersection(raycast_a, raycast_b, edges[e][0], edges[e][1]);

		if (intersection.hit) {
			const auto hit_location = intersection.intersection;
			const auto final_indicator_location = hit_location + bbox / 2;

			auto indicator_angle = 0.f;

			if (should_rotate_indicator_tex) {
				const real32 indicator_angles[4] = {
					-180,
					-90,
					0,
					90
				};

				indicator_angle = indicator_angles[e];
			}

			auto indicator_aabb = ltrb(hit_location, bbox);

			augs::detail_sprite(output, indicator_tex, final_indicator_location, indicator_angle, col);

			if (next_tex != std::nullopt) {
				const auto bbox_a = vec2(bbox);
				const auto bbox_b = vec2(next_tex->get_original_size());

				const auto x_off = bbox_b.x / 2 + bbox_a.x / 2;
				const auto y_off = bbox_b.y / 2 + bbox_a.y / 2;

				const vec2 additional_icon_offsets[4] = {
					vec2(0, y_off),
					vec2(-x_off, 0),
					vec2(0, -y_off),
					vec2(x_off, 0)
				};

				const auto next_indicator_location = vec2(final_indicator_location + additional_icon_offsets[e]);

				augs::detail_sprite(output, *next_tex, next_indicator_location, 0, col);

				indicator_aabb.contain(ltrb::center_and_size(next_indicator_location, next_tex->get_original_size()));
			}

			{
				const bool has_secondary = secondary_text.size();
				auto& primary = primary_text;
				const auto& secondary = secondary_text;

				augs::ralign_flags flags;
				auto text_pos = indicator_pos;

				if (e == 0) {
					if (indicator_pos.x < screen_size.x / 2) {
						text_pos = indicator_aabb.right_top();
					}
					else {
						text_pos = indicator_aabb.left_top();
						flags.set(augs::ralign::R);
					}

					if (has_secondary) {
						primary = primary + "\n" + secondary;
					}
				}

				if (e == 2) {
					flags.set(augs::ralign::B);

					if (indicator_pos.x < screen_size.x / 2) {
						text_pos = indicator_aabb.right_bottom();
					}
					else {
						text_pos = indicator_aabb.left_bottom();
						flags.set(augs::ralign::R);
					}

					if (has_secondary) {
						primary = secondary + "\n" + primary;
					}
				}

				if (e == 1) {
					text_pos.set(indicator_aabb.l, indicator_aabb.get_center().y);
					flags.set(augs::ralign::R);
					flags.set(augs::ralign::CY);

					if (has_secondary) {
						primary = secondary + " " + primary;
					}
				}
				if (e == 3) {
					text_pos.set(indicator_aabb.r, indicator_aabb.get_center().y);
					flags.set(augs::ralign::CY);

					if (has_secondary) {
						primary = primary + " " + secondary;
					}
				}

				const auto text = formatted_string { primary_text, { gui_font, col } };

				auto stroke_color = black;
				stroke_color.a = col.a;

				augs::gui::text::print_stroked(
					output,
					text_pos,
					text,
					flags,
					stroke_color
				);
			}

			break;
		}
	}
}
