#pragma once
#include "augs/drawing/polygon.h"
#include "augs/texture_atlas/atlas_entry.h"
#include "augs/drawing/drawing.h"
#include "augs/drawing/drawing_input_base.h"

namespace augs {
	template <std::size_t vertex_count, std::size_t index_count>
	void draw(
		const polygon<vertex_count, index_count>& poly,
		const augs::drawer output,
		const atlas_entry texture,
		const transformr target_transform
	) {
		vertex_triangle new_tri;

		auto model_transformed = poly.vertices;

		for (auto& v : model_transformed) {
			if (std::abs(target_transform.rotation) > 0.f) {
				v.pos.rotate(target_transform.rotation, vec2(0, 0));
			}

			v.pos += target_transform.pos;

			v.pos.x = static_cast<float>(static_cast<int>(v.pos.x));
			v.pos.y = static_cast<float>(static_cast<int>(v.pos.y));

			v.set_texcoord(v.texcoord, texture);
		}

		const auto& indices = poly.triangulation_indices;

		for (std::size_t i = 0; i < indices.size(); i += 3) {
			new_tri.vertices[0] = model_transformed[indices[i]];
			new_tri.vertices[1] = model_transformed[indices[i + 1]];
			new_tri.vertices[2] = model_transformed[indices[i + 2]];

			output.push(new_tri);
		}
	}
	
	template <
		class id_type,
		std::size_t vertex_count,
		std::size_t index_count,
		class M
	>
	void draw(
		const polygon_with_id<id_type, vertex_count, index_count>& poly,
		const M& manager,
		const polygon_drawing_input& in
	) {
		const auto texture = manager.at(poly.texture_map_id);

		if (in.use_neon_map) {
			using sprite_type = sprite<id_type>;

			auto neon_in = typename sprite_type::drawing_input{ in.output };

			neon_in.renderable_transform = in.renderable_transform;
			neon_in.colorize = in.colorize;

			neon_in.global_time_seconds = in.global_time_seconds;
			neon_in.use_neon_map = true;

			sprite_type neon_sprite;
			neon_sprite.set(poly.texture_map_id, texture.get_original_size());

			augs::draw(neon_sprite, manager, neon_in);

			return;
		}

		augs::draw(
			poly,
			in.output,
			texture,
			in.renderable_transform
		);
	}
}
