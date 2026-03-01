#include "rendering_scripts.h"
#include "game/cosmos/entity_handle.h"
#include "game/cosmos/cosmos.h"
#include "game/detail/entity_handle_mixins/inventory_mixin.hpp"
#include "game/components/gun_component.h"
#include "game/components/explosive_component.h"
#include "game/inferred_caches/physics_world_cache.h"
#include "game/enums/filters.h"
#include "game/detail/entity_scripts.h"
#include "game/detail/gun/gun_math.h"

#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"
#include "view/audiovisual_state/systems/interpolation_system.h"
#include "view/game_drawing_settings.h"
#include "game/detail/weapon_like.h"
#include "game/detail/crosshair_math.hpp"

#include "augs/drawing/drawing.hpp"
#include "augs/log.h"

#include "game/modes/detail/item_purchase_logic.hpp"

void draw_crosshair_procedurally(
	const crosshair_drawing_settings& settings,
	augs::drawer_with_default target,
	vec2 center,
	float recoil_amount
) {
	const auto in_col = settings.inside_color;
	const auto bo_col = settings.border_color;
	auto round_center = center;

	auto borders = border_input();
	borders.width = settings.border_width;

	if (settings.scale % 2 == 1)
	{
		round_center = vec2(vec2(center).discard_fract() + vec2(0.5f, 0.5f));
	}

	/*
		Snap a scaled dimension so that rect edges land on integer pixels.
		Odd scale puts center at half-pixel, so dimensions must be odd.
		Even scale puts center at integer, so dimensions must be even.
	*/
	auto snap = [&](float v) -> float {
		int iv = static_cast<int>(v);
		if (iv % 2 != settings.scale % 2) {
			--iv;
		}
		return static_cast<float>(std::max(iv, 1));
	};

	const auto dot_size = vec2::square(snap(1 * settings.dot_size * settings.scale));

	if (settings.show_dot)
	{
		const auto dot_origin = ltrb::center_and_size(round_center, dot_size);

		target.aabb_with_border(dot_origin, in_col, bo_col, borders);
	}

	const auto hori_segment_size = vec2(snap(settings.scale * settings.segment_length), snap(settings.scale * settings.segment_thickness));
	const auto vert_segment_size = hori_segment_size.transposed();

	float total_offset = settings.recoil_expansion_base;
	total_offset += recoil_amount * settings.recoil_expansion_mult;

	total_offset *= settings.scale;
	total_offset = std::round(total_offset);

	const auto l_segment_origin = ltrb::center_and_size(
		vec2(round_center.x - hori_segment_size.x / 2 - dot_size.x / 2 - total_offset, round_center.y),
		hori_segment_size
	);
	
	const auto r_segment_origin = ltrb::center_and_size(
		vec2(round_center.x + hori_segment_size.x / 2 + dot_size.x / 2 + total_offset, round_center.y),
		hori_segment_size
	);

	const auto t_segment_origin = ltrb::center_and_size(
		vec2(round_center.x, round_center.y - vert_segment_size.y / 2 - dot_size.y / 2- total_offset),
		vert_segment_size
	);
	
	const auto b_segment_origin = ltrb::center_and_size(
		vec2(round_center.x, round_center.y + vert_segment_size.y / 2 + dot_size.x / 2 + total_offset),
		vert_segment_size
	);

	target.aabb_with_border(l_segment_origin, in_col, bo_col, borders);
	target.aabb_with_border(r_segment_origin, in_col, bo_col, borders);
	target.aabb_with_border(t_segment_origin, in_col, bo_col, borders);
	target.aabb_with_border(b_segment_origin, in_col, bo_col, borders);
}

void draw_crosshair_circular(
	const crosshair_drawing_settings& settings,
	augs::drawer_with_default target,
	augs::special_buffer& specials,
	vec2 center,
	vec2i screen_size,
	float recoil_amount
) {
	const auto scale = static_cast<float>(settings.scale);

	float total_offset = settings.recoil_expansion_base;
	total_offset += recoil_amount * settings.recoil_expansion_mult;
	total_offset *= scale;

	const float ring_thickness = scale * settings.segment_thickness;
	const float border = static_cast<float>(settings.border_width);

	/* Y flip: convert from screen-space (top-left origin) to gl_FragCoord space (bottom-left origin) */
	const vec2 ring_center_screen(center.x, static_cast<float>(screen_size.y) - center.y);

	auto push_ring = [&](const float inner_r, const float outer_r, const rgba color) {
		if (outer_r <= 0.f) {
			return;
		}

		const auto size = vec2::square(outer_r * 2.f);
		const auto rect = ltrb::center_and_size(center, size);

		target.aabb(rect, color);

		augs::special sp;
		sp.v1 = ring_center_screen;
		sp.v2.x = inner_r;
		sp.v2.y = outer_r;

		for (int i = 0; i < 6; ++i) {
			specials.push_back(sp);
		}
	};

	const float inner_ring_outer = total_offset + ring_thickness;
	const float border_outer = inner_ring_outer + border;

	push_ring(0.f, total_offset, settings.background_color);
	push_ring(total_offset, inner_ring_outer, settings.inside_color);
	push_ring(inner_ring_outer, border_outer, settings.border_color);
}

void line_output_wrapper::operator()(const vec2 from, const vec2 to, const rgba col) const {
	output.line(line_tex, from, to, col);
}

void dashed_line_output_wrapper::operator()(const vec2 from, const vec2 to, const rgba col) const {
	output.dashed_line(line_tex, from, to, col, len, vel, global_time_seconds);
}

void draw_crosshair_lasers(const draw_crosshair_lasers_input in) {
	if (in.character.alive()) {
		const auto subject_with_crosshair = in.character; 

		const auto& cosm = in.character.get_cosmos();
		const auto& physics = cosm.get_solvable_inferred().physics;

		const vec2 crosshair_pos = in.crosshair_displacement + subject_with_crosshair.get_world_crosshair_transform(in.interpolation, false).pos;

		auto calc_color = [&](const const_entity_handle target) {
			const auto att = calc_attitude(in.character, target);

			if (att == attitude_type::WANTS_TO_KILL || att == attitude_type::WANTS_TO_KNOCK_UNCONSCIOUS) {
				return red;
			}
			else if (att == attitude_type::WANTS_TO_HEAL) {
				return green;
			}
			else {
				return cyan;
			}
		};
		
		const auto make_laser_from_to = [&](
			const const_entity_handle subject,
			const vec2 line_from,
			vec2 line_to,
			const b2Filter& filter
		) {
			auto detected_color = cyan;

			{
				const auto raycast = physics.ray_cast_px(
					cosm.get_si(),
					line_from,
					line_to,
					filter,
					subject
				);

				if (raycast.hit) {
					detected_color = calc_color(cosm[raycast.what_entity]);
				}
			}
			
			const auto laser_dir = (line_to - line_from).normalize();
			line_to += laser_dir * 10000;

			const auto raycast = physics.ray_cast_px(
				cosm.get_si(),
				line_from,
				line_to,
				filter,
				subject
			);

			if (raycast.hit) {
				in.dashed_line_callback(raycast.intersection, line_to, white);

				in.callback(
					line_from, 
					raycast.intersection, 
					detected_color
				);
			}
			else {
				in.callback(
					line_from,
					line_to,
					detected_color
				);
			}
		};

		for (const auto& subject_item_id : in.character.get_wielded_items()) {
			const auto subject_item = cosm[subject_item_id];

			std::optional<b2Filter> queried_filter;

			if (subject_item.has<components::gun>()) {
				queried_filter = ::calc_gun_bullet_physical_filter(subject_item);
			}

			if (subject_item.has<components::hand_fuse>()) {
				queried_filter = filters[predefined_filter_type::FLYING_EXPLOSIVE];
			}

			if (subject_item.has<components::melee>()) {
				queried_filter = filters[predefined_filter_type::FLYING_MELEE];
			}

			if (queried_filter.has_value()) {
				const auto rifle_transform = subject_item.get_viewing_transform(in.interpolation);
				const auto barrel_center = calc_barrel_center(subject_item, rifle_transform);
				const auto muzzle = calc_muzzle_transform(subject_item, rifle_transform).pos;

				const auto proj = crosshair_pos.get_projection_multiplier(barrel_center, muzzle);

				if (proj > 1.f) {
					const auto line_from = muzzle;
					const auto line_to = barrel_center + (muzzle - barrel_center) * proj;

					make_laser_from_to(
						subject_item,
						line_from,
						line_to,
						*queried_filter
					);
				}
			}
		}
	}
}
