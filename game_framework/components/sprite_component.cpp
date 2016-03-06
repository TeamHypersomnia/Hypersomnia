#include "sprite_component.h"
#include "texture_baker/texture_baker.h"

#include "../shared/state_for_drawing.h"
#include "../components/render_component.h"

#include "entity_system/entity.h"

#include "graphics/renderer.h"

#include "game_framework/resources/manager.h"

using namespace augs;
using namespace shared;

namespace components {
	void sprite::make_rect(vec2 pos, vec2 size, float angle, vec2 v[4]) {
		vec2 origin(pos + (size / 2.f));

		v[0] = pos;
		v[1] = pos + vec2(size.x, 0.f);
		v[2] = pos + size;
		v[3] = pos + vec2(0.f, size.y);

		v[0].rotate(angle, origin);
		v[1].rotate(angle, origin);
		v[2].rotate(angle, origin);
		v[3].rotate(angle, origin);

		v[0] -= size / 2.f;
		v[1] -= size / 2.f;
		v[2] -= size / 2.f;
		v[3] -= size / 2.f;
	}

	void sprite::set(assets::texture_id _tex, rgba _color) {
		tex = _tex;
		color = _color;

		update_size();
	}

	void sprite::update_size() {
		size = vec2i(resource_manager.find(tex)->tex.get_size());
	}

	void sprite::draw(const state_for_drawing_renderable& in) const {
		static thread_local vec2 v[4];
		assert(tex != assets::texture_id::INVALID_TEXTURE);

		vec2i transform_pos = in.renderable_transform.pos;
		make_rect(transform_pos, vec2(size), in.renderable_transform.rotation, v);

		if (in.screen_space_mode) {
			make_rect(transform_pos + vec2(size)/2, vec2(size), in.renderable_transform.rotation + rotation_offset, v);
		}
		else {
			auto center = in.visible_world_area / 2;

			auto target_position = transform_pos - in.camera_transform.pos + center + center_offset;
			make_rect(target_position, vec2(size), in.renderable_transform.rotation + rotation_offset, v);

			/* rotate around the center of the screen */
			if (in.camera_transform.rotation != 0.f)
				for (auto& vert : v)
					vert.rotate(in.camera_transform.rotation, center);
		}

		vertex_triangle t1, t2;
		t1.vertices[0].color = t2.vertices[0].color = color;
		t1.vertices[1].color = t2.vertices[1].color = color;
		t1.vertices[2].color = t2.vertices[2].color = color;

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

		if (in.renderable.alive()) {
			auto renderable = in.renderable;
			auto* render = renderable->find<components::render>();
			
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
	}

	std::vector<vec2> sprite::get_vertices() const {
		std::vector<vec2> out;
		out.push_back(size / -2.f);
		out.push_back(size / -2.f + vec2(size.x, 0.f));
		out.push_back(size / -2.f + size);
		out.push_back(size / -2.f + vec2(0.f, size.y));
		return std::move(out);
	}
	
	augs::rects::ltrb<float> sprite::get_aabb(components::transform transform) const {
		static thread_local vec2 v[4];
		make_rect(transform.pos, vec2(size), transform.rotation, v);
		return augs::rects::ltrb<float>::get_aabb(v);
	}
}
