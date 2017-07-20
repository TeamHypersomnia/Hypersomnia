#include "sprite_component.h"

#include "augs/ensure.h"
#include "augs/math/vec2.h"

#include "game/assets/assets_manager.h"
#include "game/transcendental/cosmos.h"

#include "augs/graphics/vertex.h"
#include "augs/graphics/drawers.h"

namespace components {
	void sprite::drawing_input::set_global_time_seconds(const double secs) {
		global_time_seconds = secs;
	}

	void sprite::set(
		const assets::game_image_id _tex, 
		const vec2 _size,
		const rgba _color
	) {
		overridden_size = _size;
		set(_tex, _color);
	}

	void sprite::set(
		const assets::game_image_id _tex,
		const rgba _color
	) {
		tex = _tex;
		color = _color;
	}

	void sprite::draw(
		const drawing_input in,
		const augs::texture_atlas_entry considered_texture,
		const vec2i target_position,
		const float final_rotation,
		const vec2 considered_size
	) const {
		
		auto v = augs::make_sprite_points(target_position, considered_size, final_rotation);

		/* rotate around the center of the screen */
		//if (in.camera.transform.rotation != 0.f) {
		//	const auto center = in.camera.visible_world_area / 2;
		//
		//	for (auto& vert : v) {
		//		vert.rotate(in.camera.transform.rotation, center);
		//	}
		//}

		auto target_color = color;

		if (in.colorize != white) {
			target_color *= in.colorize;
		}

		auto tris = augs::make_sprite_triangles(v, considered_texture, target_color, flip_horizontally != 0, flip_vertically != 0);

		if (effect == special_effect::COLOR_WAVE) {
			augs::vertex_triangle& t1 = tris[0], &t2 = tris[1];

			rgba left_col;
			rgba right_col;
			left_col.set_hsv({ fmod(in.global_time_seconds / 2.f, 1.f), 1.0, 1.0 });
			right_col.set_hsv({ fmod(in.global_time_seconds / 2.f + 0.3f, 1.f), 1.0, 1.0 });

			t1.vertices[0].color = t2.vertices[0].color = left_col;
			t2.vertices[1].color = right_col;
			t1.vertices[1].color = t2.vertices[2].color = right_col;
			t1.vertices[2].color = left_col;
		}

		in.target_buffer.push_back(tris[0]);
		in.target_buffer.push_back(tris[1]);
	}

	void sprite::draw_from_lt(drawing_input in) const {
		in.renderable_transform.pos += get_size(get_assets_manager()) / 2;
		draw(in);
	}

	void sprite::draw(const drawing_input in) const {
		ensure(tex != assets::game_image_id::INVALID);

		const auto& manager = get_assets_manager();

		vec2i transform_pos = in.renderable_transform.pos;
		const float final_rotation = in.renderable_transform.rotation + rotation_offset;

		auto screen_space_pos = in.camera[transform_pos];
		const auto drawn_size = get_size(manager);

		if (center_offset.non_zero()) {
			screen_space_pos -= vec2(center_offset).rotate(final_rotation, vec2(0, 0));
		}

		if (in.drawing_type == renderable_drawing_type::NEON_MAPS) {
			const auto& maybe_neon_map = manager.at(tex).texture_maps[texture_map_type::NEON];

			if (maybe_neon_map.exists()) {
				draw(
					in,
					maybe_neon_map,
					screen_space_pos,
					final_rotation,
					vec2(maybe_neon_map.original_size_pixels)
					/ manager.at(tex).get_size() * drawn_size
				);
			}
		}
		else if (
			in.drawing_type == renderable_drawing_type::NORMAL
			|| in.drawing_type == renderable_drawing_type::BORDER_HIGHLIGHTS
		) {
			draw(
				in, 
				manager.at(tex).texture_maps[texture_map_type::DIFFUSE],
				screen_space_pos,
				final_rotation, 
				drawn_size
			);
		}
		else if (in.drawing_type == renderable_drawing_type::SPECULAR_HIGHLIGHTS) {
			const auto frame_duration_ms = 50u;
			const auto frame_count = int(assets::game_image_id::BLINK_7) - int(assets::game_image_id::BLINK_1);
			const auto animation_max_duration = frame_duration_ms * frame_count;
			
			for (unsigned m = 0; m < max_specular_blinks; ++m) {
				const auto total_ms = static_cast<unsigned>(in.global_time_seconds * 1000 + (animation_max_duration / max_specular_blinks) * m);

				const auto spatial_hash = std::hash<vec2>()(in.renderable_transform.pos);

				fast_randomization generator(spatial_hash + m * 30 + total_ms / animation_max_duration);

				const unsigned animation_current_ms = total_ms % (animation_max_duration);
				const auto& target_frame = assets::game_image_id(int(assets::game_image_id::BLINK_1) + animation_current_ms / frame_duration_ms);

				const auto blink_offset = 
					vec2i { generator.randval(0, int(drawn_size.x)), generator.randval(0, int(drawn_size.y)) } - drawn_size / 2;

				draw(
					in, 
					manager.at(target_frame).texture_maps[texture_map_type::DIFFUSE],
					screen_space_pos + blink_offset,
					final_rotation,
					manager.at(target_frame).get_size()
				);
			}
		}
	}
}
