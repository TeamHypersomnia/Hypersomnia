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
		const camera_cone& cone
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

		const auto first_center_local = -vec2(tile_size * (times - vec2i(1, 1))) / 2;

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

		auto triangles = make_sprite_triangles(
			considered_texture,
			points,
			target_color, 
			in.flip 
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

	/*
		Draws the neon map in two pieces:
		- the front half stays exactly where it would normally be, so the glow's tip keeps touching the bullet's tip,
		- the back half is stretched backward so that the total length becomes neon_extension_mult times the normal one,
		  with a linear alpha gradient fading to zero at its end.

		This makes the glow overlap the bullet's particle trail instead of ending abruptly behind the bullet.
		The gradient is interpolated per-fragment by the GPU, so no banding occurs regardless of the neon map's resolution.

		Assumes the sprite faces right at zero rotation and ignores in.flip.
	*/

	template <class id_type>
	FORCE_INLINE void detail_draw_neon_extended(
		const sprite<id_type>& spr,
		const sprite_drawing_input in,
		const atlas_entry considered_texture,
		const vec2 target_position,
		const float target_rotation,
		const vec2 considered_size,
		rgba target_color
	) {
		if (in.colorize != white) {
			target_color *= in.colorize;
		}

		auto faded_color = target_color;
		faded_color.a = 0;

		const auto dir = vec2::from_degrees(target_rotation);
		const auto half_length = considered_size.x / 2;
		const auto tail_length = half_length * (2 * spr.neon_extension_mult - 1);

		const auto head_center = target_position + dir * (half_length / 2);
		const auto tail_center = target_position - dir * (tail_length / 2);

		auto push_piece = [&](
			const vec2 center,
			const vec2 size,
			const float u_from,
			const float u_to,
			const rgba back_color
		) {
			const auto points = make_sprite_points(center, size, target_rotation);

			auto t1 = vertex_triangle();
			auto t2 = vertex_triangle();

			const std::array<vec2, 4> texcoords = {
				vec2(u_from, 0.f),
				vec2(u_to, 0.f),
				vec2(u_to, 1.f),
				vec2(u_from, 1.f)
			};

			t1.vertices[0].texcoord = t2.vertices[0].texcoord = considered_texture.get_atlas_space_uv(texcoords[0]);
			t2.vertices[1].texcoord = considered_texture.get_atlas_space_uv(texcoords[1]);
			t1.vertices[1].texcoord = t2.vertices[2].texcoord = considered_texture.get_atlas_space_uv(texcoords[2]);
			t1.vertices[2].texcoord = considered_texture.get_atlas_space_uv(texcoords[3]);

			t1.vertices[0].pos = t2.vertices[0].pos = points[0];
			t2.vertices[1].pos = points[1];
			t1.vertices[1].pos = t2.vertices[2].pos = points[2];
			t1.vertices[2].pos = points[3];

			t1.vertices[0].color = t2.vertices[0].color = back_color;
			t2.vertices[1].color = target_color;
			t1.vertices[1].color = t2.vertices[2].color = target_color;
			t1.vertices[2].color = back_color;

			in.output.push(t1);
			in.output.push(t2);
		};

		push_piece(head_center, vec2(half_length, considered_size.y), 0.5f, 1.f, target_color);
		push_piece(tail_center, vec2(tail_length, considered_size.y), 0.f, 0.5f, faded_color);
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
					const auto considered_neon_size = original_neon_size * neon_size_mult;

					if (spr.neon_extension_mult > 1.f) {
						detail_draw_neon_extended(
							spr,
							in,
							maybe_neon_map,
							pos,
							final_rotation,
							considered_neon_size,
							spr.neon_color
						);
					}
					else {
						detail_draw(
							spr,
							in,
							maybe_neon_map,
							pos,
							final_rotation,
							considered_neon_size,
							spr.neon_color
						);
					}
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
					in.cone
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
