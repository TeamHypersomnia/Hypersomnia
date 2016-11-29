#include "sprite_component.h"
#include "augs/texture_baker/texture_baker.h"

#include "game/detail/state_for_drawing_camera.h"
#include "game/components/render_component.h"

#include "game/transcendental/entity_id.h"

#include "augs/graphics/renderer.h"

#include "game/resources/manager.h"

#include "augs/graphics/vertex.h"
#include "augs/ensure.h"
#include "augs/math/vec2.h"
using namespace augs;

namespace components {
	void sprite::drawing_input::setup_from(const state_for_drawing_camera& state) {
		camera = state.camera;
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

	void sprite::set(assets::texture_id _tex, rgba _color) {
		tex = _tex;
		color = _color;

		update_size_from_texture_dimensions();
	}

	void sprite::update_size_from_texture_dimensions() {
		size = vec2i(resource_manager.find(tex)->tex.get_size());
	}

	void sprite::draw(const drawing_input& in) const {
		ensure(tex != assets::texture_id::INVALID);

		const augs::texture* const original_texture = &resource_manager.find(tex)->tex;
		const augs::texture* considered_texture = original_texture;
		vec2 neon_size_multiplier = vec2(1.f, 1.f);

		if (in.use_neon_map) {
			const auto* const maybe_neon_map = resource_manager.find_neon_map(tex);

			if (maybe_neon_map != nullptr) {
				considered_texture = &maybe_neon_map->tex;
				neon_size_multiplier = vec2(considered_texture->get_size()) / vec2(original_texture->get_size());
			}
			else {
				return;
			}
		}
		
		std::array<vec2, 4> v;

		vec2i transform_pos = in.renderable_transform.pos;

		const float final_rotation = in.renderable_transform.rotation + rotation_offset;

		const auto center = in.camera.visible_world_area / 2;

		auto target_position = transform_pos - in.camera.transform.pos + center;
		
		if (center_offset.non_zero()) {
			target_position -= vec2(center_offset).rotate(final_rotation, vec2(0, 0));
		}

		const auto considered_size = get_size() * neon_size_multiplier;

		make_rect(target_position, considered_size, final_rotation, v, in.positioning);

		/* rotate around the center of the screen */
		if (in.camera.transform.rotation != 0.f) {
			for (auto& vert : v) {
				vert.rotate(in.camera.transform.rotation, center);
			}
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

		for (int i = 0; i < 3; ++i) {
			considered_texture->get_uv(t1.vertices[i].texcoord);
			considered_texture->get_uv(t2.vertices[i].texcoord);
		}

		t1.vertices[0].pos = t2.vertices[0].pos = vec2i(v[0]);
		t2.vertices[1].pos = vec2i(v[1]);
		t1.vertices[1].pos = t2.vertices[2].pos = vec2i(v[2]);
		t1.vertices[2].pos = vec2i(v[3]);

		in.target_buffer.push_back(t1);
		in.target_buffer.push_back(t2);
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
