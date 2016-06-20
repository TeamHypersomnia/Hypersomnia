#include "sprite_component.h"
#include "texture_baker/texture_baker.h"

#include "game/detail/state_for_drawing.h"
#include "game/components/render_component.h"

#include "game/entity_id.h"

#include "graphics/renderer.h"

#include "game/resources/manager.h"

#include "ensure.h"
using namespace augs;
using namespace shared;

namespace components {
	void sprite::make_rect(vec2 pos, vec2 size, float angle, vec2 v[4], bool pos_at_center) {
		vec2 origin = pos;
		
		if(pos_at_center)
			origin += size / 2.f;

		v[0] = pos;
		v[1] = pos + vec2(size.x, 0.f);
		v[2] = pos + size;
		v[3] = pos + vec2(0.f, size.y);

		v[0].rotate(angle, origin);
		v[1].rotate(angle, origin);
		v[2].rotate(angle, origin);
		v[3].rotate(angle, origin);

		if (pos_at_center) {
			v[0] -= size / 2.f;
			v[1] -= size / 2.f;
			v[2] -= size / 2.f;
			v[3] -= size / 2.f;
		}
	}

	vec2 sprite::get_size() const {
		return size * size_multiplier;
	}

	void sprite::set(assets::texture_id _tex, rgba _color) {
		tex = _tex;
		color = _color;

		update_size_from_texture_dimensions();
	}

	void sprite::update_size_from_texture_dimensions() {
		size = vec2i(resource_manager.find(tex)->tex.get_size());
	}

	void sprite::draw(const state_for_drawing_renderable& in) const {
		static thread_local vec2 v[4];
		ensure(tex != assets::texture_id::INVALID_TEXTURE);

		vec2i transform_pos = in.renderable_transform.pos;

		float final_rotation = in.renderable_transform.rotation + rotation_offset;

		if (in.screen_space_mode) {
			make_rect(transform_pos + get_size()/2, get_size(), final_rotation, v, true);
		}
		else {
			auto center = in.visible_world_area / 2;

			auto target_position = transform_pos - in.camera_transform.pos + center;
			
			if (center_offset.non_zero())
				target_position -= vec2(center_offset).rotate(final_rotation, vec2(0, 0));

			make_rect(target_position, get_size(), final_rotation, v, !in.position_is_left_top_corner);

			/* rotate around the center of the screen */
			if (in.camera_transform.rotation != 0.f)
				for (auto& vert : v)
					vert.rotate(in.camera_transform.rotation, center);
		}

		vertex_triangle t1, t2;

		auto target_color = color;

		if (in.colorize != augs::white) {
			target_color *= in.colorize;
		}

		t1.vertices[0].color = t2.vertices[0].color = target_color;
		t1.vertices[1].color = t2.vertices[1].color = target_color;
		t1.vertices[2].color = t2.vertices[2].color = target_color;

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

		auto& texture = resource_manager.find(tex)->tex;

		for (int i = 0; i < 3; ++i) {
			texture.get_uv(t1.vertices[i].texcoord);
			texture.get_uv(t2.vertices[i].texcoord);
		}

		t1.vertices[0].pos = t2.vertices[0].pos = vec2i(v[0]);
		t2.vertices[1].pos = vec2i(v[1]);
		t1.vertices[1].pos = t2.vertices[2].pos = vec2i(v[2]);
		t1.vertices[2].pos = vec2i(v[3]);

		components::render* render = nullptr;

		if (in.renderable.alive()) {
			auto renderable = in.renderable;
			render = renderable.find<components::render>();
			
			if (render != nullptr) {
				/* compute average */
				render->last_screen_pos = (vec2(v[0]) + vec2(v[1]) + vec2(v[2]) + vec2(v[3])) / 4;
				render->was_drawn = true;
			}
		}
	
		if (in.overridden_target_buffer) {
			in.overridden_target_buffer->push_back(t1);
			in.overridden_target_buffer->push_back(t2);
		}
		else {
			in.output->push_triangle(t1);
			in.output->push_triangle(t2);
		}

		if (render != nullptr && render->partial_overlay_height_ratio > 0.f) {
			size_t lower_left, lower_right = 0, upper_left = 0, upper_right = 0;

			upper_left = std::min_element(v, v + 4) - v;
			upper_right = (upper_left + 1) % 4;
			lower_right = (upper_right + 1) % 4;
			lower_left = (lower_right + 1) % 4;

			auto ratio = render->partial_overlay_height_ratio;
			auto col = render->partial_overlay_color;

			t1.vertices[0].color = t2.vertices[0].color = col;
			t1.vertices[1].color = t2.vertices[1].color = col;
			t1.vertices[2].color = t2.vertices[2].color = col;

			v[upper_left] = augs::interp(v[lower_left], v[upper_left], ratio);
			v[upper_right] = augs::interp(v[lower_right], v[upper_right], ratio);

			//if (v[upper_right].y > v[upper_left].y) {
			//	v[upper_right].y = v[upper_left].y;
			//}
			//if (v[upper_left].y > v[upper_right].y) {
			//	v[upper_left].y = v[upper_right].y;
			//}

			texcoords[upper_left] = augs::interp(texcoords[lower_left], texcoords[upper_left], ratio);
			texcoords[upper_right] = augs::interp(texcoords[lower_right], texcoords[upper_right], ratio);

			t1.vertices[0].pos = t2.vertices[0].pos = vec2i(v[0]);
			t2.vertices[1].pos = vec2i(v[1]);
			t1.vertices[1].pos = t2.vertices[2].pos = vec2i(v[2]);
			t1.vertices[2].pos = vec2i(v[3]);

			t1.vertices[0].texcoord = t2.vertices[0].texcoord = texcoords[0];
			t2.vertices[1].texcoord = texcoords[1];
			t1.vertices[1].texcoord = t2.vertices[2].texcoord = texcoords[2];
			t1.vertices[2].texcoord = texcoords[3];

			for (int i = 0; i < 3; ++i) {
				texture.get_uv(t1.vertices[i].texcoord);
				texture.get_uv(t2.vertices[i].texcoord);
			}

			if (in.overridden_target_buffer) {
				in.overridden_target_buffer->push_back(t1);
				in.overridden_target_buffer->push_back(t2);
			}
			else {
				in.output->push_triangle(t1);
				in.output->push_triangle(t2);
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
	
	augs::rects::ltrb<float> sprite::get_aabb(components::transform transform, bool screen_space_mode) const {
		static thread_local vec2 v[4];		
		
		if (screen_space_mode)
			make_rect(transform.pos + get_size() / 2, get_size(), transform.rotation + rotation_offset, v, true);
		else
			make_rect(transform.pos, get_size(), transform.rotation + rotation_offset, v, true);

		return augs::get_aabb(v);
	}
}
