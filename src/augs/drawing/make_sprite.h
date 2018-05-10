#pragma once
#include <array>
#include "augs/math/vec2.h"
#include "augs/graphics/vertex.h"
#include "augs/graphics/rgba.h"
#include "augs/build_settings/platform_defines.h"
#include "augs/drawing/flip.h"

namespace augs {
	FORCE_INLINE auto make_sprite_points(
		const vec2 pos, 
		const vec2i size, 
		const float rotation_degrees
	) {
		std::array<vec2, 4> v;

		v[0] = v[1] = v[2] = v[3] = pos - (size / 2);

		// v[0];
		v[1].x += size.x;

		v[2].x += size.x;
		v[2].y += size.y;

		v[3].y += size.y;

		v[0].rotate(rotation_degrees, pos);
		v[1].rotate(rotation_degrees, pos);
		v[2].rotate(rotation_degrees, pos);
		v[3].rotate(rotation_degrees, pos);

		return v;
	}

	FORCE_INLINE std::array<vertex_triangle, 2> make_sprite_triangles(
		const augs::atlas_entry considered_texture,
		const std::array<vec2, 4> v,
		const rgba col = white,
		const flip_flags flip = flip_flags()
	) {
		auto t1 = vertex_triangle();
		auto t2 = vertex_triangle();

		std::array<vec2, 4> texcoords = {
			vec2(0.f, 0.f),
			vec2(1.f, 0.f),
			vec2(1.f, 1.f),
			vec2(0.f, 1.f)
		};

		if (flip.horizontally) {
			for (auto& v : texcoords) {
				v.x = 1.f - v.x;
			}
		}
		if (flip.vertically) {
			for (auto& v : texcoords) {
				v.y = 1.f - v.y;
			}
		}

		t1.vertices[0].texcoord = t2.vertices[0].texcoord = texcoords[0];
		t2.vertices[1].texcoord = texcoords[1];
		t1.vertices[1].texcoord = t2.vertices[2].texcoord = texcoords[2];
		t1.vertices[2].texcoord = texcoords[3];

		for (int i = 0; i < 3; ++i) {
			t1.vertices[i].texcoord = considered_texture.get_atlas_space_uv(t1.vertices[i].texcoord);
			t2.vertices[i].texcoord = considered_texture.get_atlas_space_uv(t2.vertices[i].texcoord);
		}

		t1.vertices[0].pos = t2.vertices[0].pos = v[0];
		t2.vertices[1].pos = v[1];
		t1.vertices[1].pos = t2.vertices[2].pos = v[2];
		t1.vertices[2].pos = v[3];

		t1.vertices[0].color = t2.vertices[0].color = col;
		t1.vertices[1].color = t2.vertices[1].color = col;
		t1.vertices[2].color = t2.vertices[2].color = col;

		return { t1, t2 };
	}
}
