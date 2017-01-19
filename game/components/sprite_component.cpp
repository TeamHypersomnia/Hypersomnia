#include "sprite_component.h"
#include "augs/texture_baker/texture_baker.h"

#include "game/components/render_component.h"

#include "game/transcendental/entity_id.h"

#include "augs/graphics/renderer.h"

#include "game/resources/manager.h"

#include "augs/graphics/vertex.h"
#include "augs/graphics/drawers.h"
#include "augs/ensure.h"
#include "augs/math/vec2.h"
using namespace augs;

namespace components {
	void sprite::drawing_input::set_global_time_seconds(const float secs) {
		global_time_seconds = secs;
	}

	vec2 sprite::get_size() const {
		return size * size_multiplier;
	}

	void sprite::set(const assets::texture_id _tex, const rgba _color) {
		tex = _tex;
		color = _color;
		has_neon_map = get_resource_manager().find_neon_map(tex) != nullptr;

		update_size_from_texture_dimensions();
	}

	void sprite::update_size_from_texture_dimensions() {
		size = vec2i(get_resource_manager().find(tex)->tex.get_size());
	}

	void sprite::draw(const drawing_input& in,
		const augs::texture& considered_texture,
		const vec2i target_position,
		const float final_rotation,
		const vec2 considered_size) const {
		
		auto v = augs::make_sprite_points(target_position, considered_size, final_rotation, in.positioning);

		/* rotate around the center of the screen */
		//if (in.camera.transform.rotation != 0.f) {
		//	const auto center = in.camera.visible_world_area / 2;
		//
		//	for (auto& vert : v) {
		//		vert.rotate(in.camera.transform.rotation, center);
		//	}
		//}

		auto target_color = color;

		if (in.colorize != augs::white) {
			target_color *= in.colorize;
		}

		auto tris = augs::make_sprite_triangles(v, considered_texture, target_color, flip_horizontally != 0, flip_vertically != 0);

		if (effect == special_effect::COLOR_WAVE) {
			vertex_triangle& t1 = tris[0], &t2 = tris[1];

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

	void sprite::draw(const drawing_input& in) const {
		ensure(tex != assets::texture_id::INVALID);

		vec2i transform_pos = in.renderable_transform.pos;
		const float final_rotation = in.renderable_transform.rotation + rotation_offset;

		auto screen_space_pos = in.camera[transform_pos];
		const auto drawn_size = size*size_multiplier;

		if (center_offset.non_zero()) {
			screen_space_pos -= vec2(center_offset).rotate(final_rotation, vec2(0, 0));
		}

		if (in.drawing_type == renderable_drawing_type::NEON_MAPS && has_neon_map) {
			draw(in, get_resource_manager().find_neon_map(tex)->tex, 
				screen_space_pos,
				final_rotation,
				vec2(get_resource_manager().find_neon_map(tex)->tex.get_size())
				/ vec2(get_resource_manager().find(tex)->tex.get_size()) * drawn_size);
		}
		else if (in.drawing_type == renderable_drawing_type::NORMAL) {
			draw(in, get_resource_manager().find(tex)->tex,
				screen_space_pos,
				final_rotation, 
				drawn_size);
		}
		else if (in.drawing_type == renderable_drawing_type::SPECULAR_HIGHLIGHTS) {
			const auto& anim = *get_resource_manager().find(assets::animation_id::BLINK_ANIMATION);
			const auto frame_duration_ms = static_cast<unsigned>(anim.frames[0].duration_milliseconds);
			const auto frame_count = anim.frames.size();
			const auto animation_max_duration = frame_duration_ms * frame_count;

			for (unsigned m = 0; m < max_specular_blinks; ++m) {
				const auto total_ms = static_cast<unsigned>(in.global_time_seconds * 1000 + (animation_max_duration / max_specular_blinks) * m);

				const auto spatial_hash = std::hash<vec2>()(in.renderable_transform.pos);

				std::minstd_rand0 generator(spatial_hash + m*30 + total_ms / animation_max_duration);

				const unsigned animation_current_ms = total_ms % (animation_max_duration);
				const auto& target_frame = anim.frames[animation_current_ms / frame_duration_ms];
				
				vec2i blink_offset = { int(generator() % unsigned(drawn_size.x)), int(generator() % unsigned(drawn_size.y)) };
				blink_offset -= drawn_size / 2;

				draw(in, get_resource_manager().find(target_frame.sprite.tex)->tex,
					screen_space_pos + blink_offset,
					final_rotation,
					assets::get_size(target_frame.sprite.tex));
			}
		}
	}

	std::vector<vec2> sprite::get_vertices() const {
		std::vector<vec2> out;
		out.push_back(get_size() / -2.f);
		out.push_back(get_size() / -2.f + vec2(get_size().x, 0.f));
		out.push_back(get_size() / -2.f + get_size());
		out.push_back(get_size() / -2.f + vec2(0.f, get_size().y));
		return std::move(out);
	}
	
	ltrb sprite::get_aabb(const components::transform& transform, const renderable_positioning_type positioning) const {
		return augs::get_aabb(make_sprite_points(transform.pos, get_size(), transform.rotation + rotation_offset, positioning));
	}
}
