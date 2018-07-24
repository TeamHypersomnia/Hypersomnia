#pragma once
#include "augs/texture_atlas/atlas_entry.h"
#include "augs/math/vec2.h"
#include "augs/graphics/vertex.h"
#include "augs/graphics/rgba.h"
#include "augs/drawing/make_sprite.h"

namespace augs {
	template <class T>
	void detail_sprite(
		vertex_triangle_buffer& output_buffer,
		const T& entry,
		const vec2i size,
		const vec2 pos,
		const float rotation_degrees,
		const rgba color
	) {
		const auto considered_texture = static_cast<augs::atlas_entry>(entry);
		const auto points = make_sprite_points(pos, size, rotation_degrees);
		const auto triangles = make_sprite_triangles(considered_texture, points, color);

		output_buffer.push_back(triangles[0]);
		output_buffer.push_back(triangles[1]);
	}

	template <class T>
	void detail_sprite(
		vertex_triangle_buffer& output_buffer,
		const T& entry,
		const vec2 pos,
		const float rotation_degrees,
		const rgba color
	) {
		const auto considered_texture = static_cast<augs::atlas_entry>(entry);
		const auto points = make_sprite_points(pos, considered_texture.get_original_size(), rotation_degrees);
		const auto triangles = make_sprite_triangles(considered_texture, points, color);

		output_buffer.push_back(triangles[0]);
		output_buffer.push_back(triangles[1]);
	}

	template <class T>
	void detail_neon_sprite(
		vertex_triangle_buffer& output_buffer,
		const T& entry,
		const vec2i size,
		const vec2 pos,
		const float rotation_degrees,
		const rgba color
	) {
		if (const auto considered_texture = entry.neon_map;
			considered_texture.exists()
		) {
			const auto drawn_size = 
				vec2(considered_texture.get_original_size()) / entry.diffuse.get_original_size() * size
			;

			const auto points = make_sprite_points(pos, drawn_size, rotation_degrees);
			const auto triangles = make_sprite_triangles(considered_texture, points, color);

			output_buffer.push_back(triangles[0]);
			output_buffer.push_back(triangles[1]);
		}
	}

	template <class T>
	void detail_neon_sprite(
		vertex_triangle_buffer& output_buffer,
		const T& entry,
		const vec2 pos,
		const float rotation_degrees,
		const rgba color
	) {
		if (const auto considered_texture = entry.neon_map;
			considered_texture.exists()
		) {
			const auto drawn_size = vec2(considered_texture.get_original_size());

			const auto points = make_sprite_points(pos, drawn_size, rotation_degrees);
			const auto triangles = make_sprite_triangles(considered_texture, points, color);

			output_buffer.push_back(triangles[0]);
			output_buffer.push_back(triangles[1]);
		}
	}
}
