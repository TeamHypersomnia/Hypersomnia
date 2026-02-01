#pragma once
#include "augs/templates/traits/function_traits.h"
#include "augs/drawing/drawing.h"
#include "augs/drawing/drawing_input_base.h"
#include "augs/drawing/make_sprite.h"
#include "augs/texture_atlas/atlas_entry.h"
#include "augs/build_settings/compiler_defines.h"
#include "augs/drawing/sprite.h"
#include "augs/math/grid_math.h"
#include "augs/drawing/make_sprite_points.h"

namespace augs {
	template <class I, class F>
	FORCE_INLINE void for_each_tile(
		const sprite<I>& spr, 
		const vec2 pos,
		F callback, 
		const real32 final_rotation, 
		const vec2i tile_size,
		const camera_cone& cone,
		const vec2 tile_offset = vec2::zero
	) {
		const auto total_size = spr.get_size();
		const auto times = total_size / tile_size;

		const auto vis = [&]() {
			/* Rotate the tiled sprite so that it becomes axis aligned */
			const auto rotated_pos = vec2(pos).rotate(-final_rotation, cone.eye.transform.pos);
			auto rotated_camera = cone;

			/* Rotate the camera to the same space */
			rotated_camera.eye.transform.rotation -= final_rotation;

			const auto camera_aabb = ltrbi(rotated_camera.get_visible_world_rect_aabb());
			const auto tile_grid_aabb = ltrbi(vec2i(rotated_pos) - total_size / 2, total_size);

			return augs::visible_grid_cells_detail(
				tile_size, 
				times, 
				tile_grid_aabb, 
				camera_aabb
			);
		}();

		/* 
		 * Calculate the local offset for where the tile grid should start.
		 * For destructible sprite chunks, tile_offset represents how far into 
		 * the original sprite this chunk starts, which shifts where tiles begin.
		 */
		const auto offset_in_tiles = vec2i(
			tile_offset.x > 0 ? static_cast<int>(std::fmod(tile_offset.x, tile_size.x)) : 0,
			tile_offset.y > 0 ? static_cast<int>(std::fmod(tile_offset.y, tile_size.y)) : 0
		);

		const auto first_center_local = -vec2(tile_size * (times - vec2i(1, 1))) / 2 - vec2(offset_in_tiles);

		const auto first_center = pos + vec2(first_center_local).rotate(final_rotation);
		const auto first_lt = pos + (first_center_local - tile_size / 2).rotate(final_rotation);

		const auto dir = vec2::from_degrees(final_rotation);
		const auto dir_perp = dir.perpendicular_cw();

		const auto tile_off = dir * tile_size.x;
		const auto tile_off_perp = dir_perp * tile_size.y;
	   
		for (int y = vis.t; y < vis.b; ++y) {
			for (int x = vis.l; x < vis.r; ++x) {
				using T = argument_t<F, 0>;

				if constexpr(std::is_same_v<T, vec2>) {
					const auto x_off = tile_off * x;
					const auto y_off = tile_off_perp * y;

					callback(first_center + x_off + y_off);

					(void)first_lt;
				}
				else {
					const auto lt = first_lt + tile_off * x + tile_off_perp * y;
					const auto rt = first_lt + tile_off * (x + 1) + tile_off_perp * y;
					const auto rb = first_lt + tile_off * (x + 1) + tile_off_perp * (y + 1);
					const auto lb = first_lt + tile_off * x + tile_off_perp * (y + 1);

					callback({lt, rt, rb, lb});

					(void)first_center;
				}
			}
		}
	}


	template <class id_type>
	FORCE_INLINE void detail_draw(
		const sprite<id_type>& spr,
		const sprite_drawing_input in,
		const atlas_entry considered_texture,
		const sprite_points& points,
		rgba target_color
	) {
		if (in.colorize != white) {
			target_color *= in.colorize;
		}

		/* 
		 * For tiled sprites (where tile_size is set), use full 0-1 UV range.
		 * For non-tiled sprites, use texture_rect for UV mapping (destructible sprites).
		 */
		const bool is_tiled = in.tile_size.x > 0 && in.tile_size.y > 0;
		const auto effective_texture_rect = is_tiled ? xywh(0, 0, 1.0f, 1.0f) : in.texture_rect;

		auto triangles = make_sprite_triangles(
			considered_texture,
			points,
			target_color, 
			in.flip,
			effective_texture_rect
		);

		if (!in.disable_special_effects && spr.effect == sprite_special_effect::COLOR_WAVE) {
			auto left_col = rgba(hsv{ std::fmod(in.global_time_seconds * spr.effect_speed_multiplier / 2.f, 1.f), 1.0, 1.0 });
			auto right_col = rgba(hsv{ std::fmod(in.global_time_seconds * spr.effect_speed_multiplier / 2.f / 2.f + 0.3f, 1.f), 1.0, 1.0 });

			left_col.avoid_dark_blue_for_color_wave();
			right_col.avoid_dark_blue_for_color_wave();

			left_col.a = target_color.a;
			right_col.a = target_color.a;

			auto& t1 = triangles[0];
			auto& t2 = triangles[1];

			t1.vertices[0].color = t2.vertices[0].color = left_col;
			t2.vertices[1].color = right_col;
			t1.vertices[1].color = t2.vertices[2].color = right_col;
			t1.vertices[2].color = left_col;
		}

		in.output.push(triangles[0]);
		in.output.push(triangles[1]);
	}

	template <class id_type>
	FORCE_INLINE void detail_draw(
		const sprite<id_type>& spr,
		const sprite_drawing_input in,
		const atlas_entry considered_texture,
		const vec2 target_position,
		float target_rotation,
		const sprite_size_type considered_size,
		rgba target_color
	) {
		detail_draw(
			spr,
			in,
			considered_texture,
			make_sprite_points(target_position, considered_size, target_rotation),
			target_color
		);
	}

	template <class id_type, class M>
	FORCE_INLINE void draw(
		const sprite<id_type>& spr,
		const M& manager,
		const sprite_drawing_input in
	) {
		static_assert(
			!has_member_find_v<M, id_type>,
			"Here we assume it is always found, or a harmless default returned."
		);

		const auto pos = in.renderable_transform.pos;

		auto final_rotation = in.renderable_transform.rotation; //+ rotation_offset;

		if (spr.effect == sprite_special_effect::CONTINUOUS_ROTATION) {
			final_rotation += std::fmod(in.global_time_seconds * spr.effect_speed_multiplier * 360.f, 360.f);
		}

		const auto drawn_size = spr.get_size();

		const auto& entry = manager.at(spr.image_id);
		const auto& diffuse = entry.diffuse;

		if (in.use_neon_map) {
			const auto& maybe_neon_map = entry.neon_map;

			if (maybe_neon_map.exists()) {
				const auto original_neon_size = vec2(maybe_neon_map.get_original_size());

				if (spr.tile_excess_size && drawn_size.x >= in.tile_size.x && drawn_size.y >= in.tile_size.y) {
					const auto neon_size_mult = vec2(in.tile_size) / diffuse.get_original_size();

					for_each_tile(
						spr,
						pos,
						[&](const vec2 piece_pos) {
							detail_draw(
								spr,
								in,
								maybe_neon_map,
								piece_pos,
								final_rotation,
								original_neon_size * neon_size_mult,
								spr.neon_color
							);
						},
						final_rotation,
						in.tile_size,
						in.cone
					);
				}
				else {
					const auto original_size = vec2i(diffuse.get_original_size());
					const auto neon_size_mult = vec2(drawn_size) / original_size;

					detail_draw(
						spr,
						in,
						maybe_neon_map,
						pos,
						final_rotation,
						original_neon_size * neon_size_mult,
						spr.neon_color
					);
				}
			}
		}
		else {
			if (spr.tile_excess_size && drawn_size.x >= in.tile_size.x && drawn_size.y >= in.tile_size.y) {
				for_each_tile(
					spr,
					pos,
					[&](const sprite_points& piece_points) {
						detail_draw(
							spr,
							in,
							diffuse,
							piece_points,
							spr.color
						);
					},
					final_rotation,
					in.tile_size,
					in.cone,
					in.tile_offset
				);
			}
			else {
				detail_draw(
					spr,
					in,
					diffuse,
					pos,
					final_rotation,
					drawn_size,
					spr.color
				);
			}
		}
	}
}
