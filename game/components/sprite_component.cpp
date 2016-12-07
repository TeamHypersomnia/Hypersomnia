#include "sprite_component.h"
#include "augs/texture_baker/texture_baker.h"

#include "game/components/render_component.h"

#include "game/transcendental/entity_id.h"

#include "augs/graphics/renderer.h"

#include "game/resources/manager.h"

#include "augs/graphics/vertex.h"
#include "augs/ensure.h"
#include "augs/math/vec2.h"
using namespace augs;

namespace components {
	void sprite::drawing_input::set_global_time_seconds(const float secs) {
		global_time_seconds = secs;
	}

	void sprite::make_rect(const vec2 pos, const vec2 size, const float angle, std::array<vec2, 4>& v, const drawing_input::positioning_type positioning) {
		if (positioning == drawing_input::positioning_type::CENTER) {
			const vec2 origin = pos;
			const vec2 half_size = size / 2.f;

			v[0] = pos - half_size;
			v[1] = pos + vec2(size.x, 0.f) - half_size;
			v[2] = pos + size - half_size;
			v[3] = pos + vec2(0.f, size.y) - half_size;

			v[0].rotate(angle, origin);
			v[1].rotate(angle, origin);
			v[2].rotate(angle, origin);
			v[3].rotate(angle, origin);
		}
		else {
			const vec2 origin = pos + size / 2.f;

			v[0] = pos;
			v[1] = pos + vec2(size.x, 0.f);
			v[2] = pos + size;
			v[3] = pos + vec2(0.f, size.y);

			v[0].rotate(angle, origin);
			v[1].rotate(angle, origin);
			v[2].rotate(angle, origin);
			v[3].rotate(angle, origin);
		}
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
		const augs::texture_with_image* const considered_texture,
		const vec2i target_position,
		const float final_rotation,
		const vec2 considered_size) const {
		std::array<vec2, 4> v;

		make_rect(target_position, considered_size, final_rotation, v, in.positioning);

		/* rotate around the center of the screen */
		if (in.camera.transform.rotation != 0.f) {
			const auto center = in.camera.visible_world_area / 2;

			for (auto& vert : v) {
				vert.rotate(in.camera.transform.rotation, center);
			}
		}

		vertex_triangle t1, t2;

		auto target_color = color;

		if (in.colorize != augs::white) {
			target_color *= in.colorize;
		}

		if (effect == special_effect::COLOR_WAVE) {
			rgba left_col;
			rgba right_col;
			left_col.set_hsv({ fmod(in.global_time_seconds / 2.f, 1.f), 1.0, 1.0 });
			right_col.set_hsv({ fmod(in.global_time_seconds / 2.f + 0.3f, 1.f), 1.0, 1.0 });

			t1.vertices[0].color = t2.vertices[0].color = left_col;
			t2.vertices[1].color = right_col;
			t1.vertices[1].color = t2.vertices[2].color = right_col;
			t1.vertices[2].color = left_col;
		}
		else {
			t1.vertices[0].color = t2.vertices[0].color = target_color;
			t1.vertices[1].color = t2.vertices[1].color = target_color;
			t1.vertices[2].color = t2.vertices[2].color = target_color;
		}

		vec2 texcoords[] = {
			vec2(0.f, 0.f),
			vec2(1.f, 0.f),
			vec2(1.f, 1.f),
			vec2(0.f, 1.f)
		};

		if (flip_horizontally)
			for (auto& v : texcoords)
				v.x = 1.f - v.x;
		if (flip_vertically)
			for (auto& v : texcoords)
				v.y = 1.f - v.y;

		t1.vertices[0].texcoord = t2.vertices[0].texcoord = texcoords[0];
		t2.vertices[1].texcoord = texcoords[1];
		t1.vertices[1].texcoord = t2.vertices[2].texcoord = texcoords[2];
		t1.vertices[2].texcoord = texcoords[3];

		for (int i = 0; i < 3; ++i) {
			considered_texture->tex.get_uv(t1.vertices[i].texcoord);
			considered_texture->tex.get_uv(t2.vertices[i].texcoord);
		}

		t1.vertices[0].pos = t2.vertices[0].pos = vec2i(v[0]);
		t2.vertices[1].pos = vec2i(v[1]);
		t1.vertices[1].pos = t2.vertices[2].pos = vec2i(v[2]);
		t1.vertices[2].pos = vec2i(v[3]);

		in.target_buffer.push_back(t1);
		in.target_buffer.push_back(t2);
	}

	void sprite::draw(const drawing_input& in) const {
		ensure(tex != assets::texture_id::INVALID);

		vec2i transform_pos = in.renderable_transform.pos;
		const float final_rotation = in.renderable_transform.rotation + rotation_offset;

		const auto center = in.camera.visible_world_area / 2;

		auto target_position = transform_pos - in.camera.transform.pos + center;
		const auto drawn_size = size*size_multiplier;

		if (center_offset.non_zero()) {
			target_position -= vec2(center_offset).rotate(final_rotation, vec2(0, 0));
		}

		if (in.drawing_type == renderable_drawing_type::NEON_MAPS && has_neon_map) {
			draw(in, get_resource_manager().find_neon_map(tex), 
				target_position,
				final_rotation,
				vec2(get_resource_manager().find_neon_map(tex)->tex.get_size())
				/ vec2(get_resource_manager().find(tex)->tex.get_size()) * drawn_size);
		}
		else if (in.drawing_type == renderable_drawing_type::NORMAL) {
			draw(in, get_resource_manager().find(tex),
				target_position,
				final_rotation, 
				drawn_size);
		}
		else if (in.drawing_type == renderable_drawing_type::SPECULAR_HIGHLIGHTS) {
			const auto& anim = *get_resource_manager().find(assets::animation_id::BLINK_ANIMATION);
			const unsigned frame_duration_ms = anim.frames[0].duration_milliseconds;
			const auto frame_count = anim.frames.size();
			const auto animation_max_duration = frame_duration_ms * frame_count;

			for (unsigned m = 0; m < max_specular_blinks; ++m) {
				unsigned total_ms = in.global_time_seconds * 1000 + (animation_max_duration / max_specular_blinks) * m;

				const auto spatial_hash = ((std::hash<float>()(in.renderable_transform.pos.x)
					^ (std::hash<float>()(in.renderable_transform.pos.y) << 1)) >> 1);

				std::minstd_rand0 generator(spatial_hash + m*30 + total_ms / animation_max_duration);

				const unsigned animation_current_ms = total_ms % (animation_max_duration);
				const auto& target_frame = anim.frames[animation_current_ms / frame_duration_ms];
				
				vec2i blink_offset = { int(generator() % unsigned(drawn_size.x)), int(generator() % unsigned(drawn_size.y)) };
				blink_offset -= drawn_size / 2;

				draw(in, get_resource_manager().find(target_frame.sprite.tex),
					target_position + blink_offset,
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
	
	augs::rects::ltrb<float> sprite::get_aabb(const components::transform& transform, const drawing_input::positioning_type positioning) const {
		std::array<vec2, 4> v;		
		
		make_rect(transform.pos, get_size(), transform.rotation + rotation_offset, v, positioning);

		return augs::get_aabb(v);
	}
}
