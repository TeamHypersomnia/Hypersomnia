#pragma once
#include <array>
#include "augs/math/vec2.h"
#include "augs/math/rects.h"
#include "augs/graphics/vertex.h"
#include "augs/graphics/rgba.h"
#include "augs/build_settings/compiler_defines.h"

#include "augs/texture_atlas/atlas_entry.h"
#include "augs/drawing/flip.h"
#include "augs/drawing/make_sprite_points.h"

namespace augs {
	/*
	 * Creates sprite triangles with optional texture_rect for destructible sprites.
	 * texture_rect is in 0-1 space and defines which portion of the texture to render.
	 */
	FORCE_INLINE std::array<vertex_triangle, 2> make_sprite_triangles(
		const augs::atlas_entry considered_texture,
		const std::array<vec2, 4>& v,
		const rgba col = white,
		const flip_flags flip = flip_flags(),
		const xywh texture_rect = xywh(0, 0, 1.0f, 1.0f)
	) {
		auto t1 = vertex_triangle();
		auto t2 = vertex_triangle();

		/* 
		 * Start with texture_rect coordinates instead of 0-1.
		 * This allows destructible sprites to render only a portion of the texture.
		 */
		std::array<vec2, 4> texcoords = {
			vec2(texture_rect.x, texture_rect.y),
			vec2(texture_rect.x + texture_rect.w, texture_rect.y),
			vec2(texture_rect.x + texture_rect.w, texture_rect.y + texture_rect.h),
			vec2(texture_rect.x, texture_rect.y + texture_rect.h)
		};

		if (flip.horizontally) {
			for (auto& tc : texcoords) {
				/* Flip within the texture_rect bounds */
				tc.x = texture_rect.x + texture_rect.w - (tc.x - texture_rect.x);
			}
		}
		if (flip.vertically) {
			for (auto& tc : texcoords) {
				/* Flip within the texture_rect bounds */
				tc.y = texture_rect.y + texture_rect.h - (tc.y - texture_rect.y);
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

	FORCE_INLINE void write_sprite_triangles(
		vertex_triangle& t1,
		vertex_triangle& t2,
		const augs::atlas_entry considered_texture,
		const std::array<vec2, 4>& v,
		const rgba col = white,
		const flip_flags flip = flip_flags()
	) {
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
	}
}
