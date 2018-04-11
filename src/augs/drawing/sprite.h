#pragma once
#include "augs/pad_bytes.h"

#include "augs/ensure.h"

#include "augs/math/vec2.h"
#include "augs/math/rects.h"

#include "augs/templates/introspect_declaration.h"
#include "augs/templates/container_traits.h"

#include "augs/graphics/vertex.h"

#include "augs/drawing/drawing_input_base.h"
#include "augs/drawing/drawing.h"

#include "augs/texture_atlas/texture_atlas_entry.h"
#include "augs/build_settings/platform_defines.h"

namespace augs {
	enum class sprite_special_effect /* : unsigned char */ {
		// GEN INTROSPECTOR enum class augs::sprite_special_effect
		NONE = 0,
		COLOR_WAVE
		// END GEN INTROSPECTOR
	};

	template <class id_type>
	struct sprite {
		static constexpr bool reinfer_when_tweaking = true;
		using size_type = vec2i;

		struct drawing_input : drawing_input_base {
			using drawing_input_base::drawing_input_base;
			
			double global_time_seconds = 0.0;
			flip_flags flip;
		};

		sprite(
			const id_type tex = id_type::INVALID,
			const size_type size = size_type(),
			const rgba color = white
		) :
			tex(tex),
			size(size),
			color(color)
		{}

		template <class M>
		sprite(
			const id_type tex,
			const M& manager,
			const rgba color = rgba()
		) {
			set(tex, manager, color);
		}

		template <class M>
		void set(
			const id_type tex,
			const M& manager,
			const rgba color = rgba()
		) {
			this->tex = tex;
			this->size = manager.at(tex).get_size();
			this->color = color;
		}

		template <class T>
		void set(
			const id_type tex,
			const basic_vec2<T> size,
			const rgba color = rgba()
		) {
			this->tex = tex;
			this->size = size;
			this->color = color;
		}
		
		void set_color(const rgba col) {
			color = col;
		}

		// GEN INTROSPECTOR struct augs::sprite class id_type
		id_type tex = id_type::INVALID;
		rgba color;
		size_type size;
		sprite_special_effect effect = sprite_special_effect::NONE;
		// END GEN INTROSPECTOR

		bool operator==(const sprite& b) const {
			return recursive_equal(*this, b);
		}

		vec2i get_size() const {
			return size;
		}

		ltrb get_aabb(const transform transform) const {
			return augs::get_aabb(
				make_sprite_points(
					transform.pos, 
					get_size(),
					transform.rotation //+ rotation_offset
				)
			);
		}
		
		template <class M>
		FORCE_INLINE void draw(
			const M& manager,
			const drawing_input in
		) const {
			static_assert(
				!has_member_find_v<M, id_type>,
			   	"Here we assume it is always found, or a harmless default returned."
			);

			const auto transform_pos = in.renderable_transform.pos;
			const auto final_rotation = in.renderable_transform.rotation; //+ rotation_offset;
			const auto drawn_size = get_size();

			if (in.use_neon_map) {
				const auto& maybe_neon_map = manager.at(tex).neon_map;

				if (maybe_neon_map.exists()) {
					draw(
						in,
						maybe_neon_map,
						transform_pos,
						final_rotation,
						vec2(maybe_neon_map.get_size())
						/ manager.at(tex).get_size() * drawn_size
					);
				}
			}
			else {
				draw(
					in,
					manager.at(tex).diffuse,
					transform_pos,
					final_rotation,
					drawn_size
				);
			}
		}
		
		FORCE_INLINE void draw(
			const drawing_input in,
			const texture_atlas_entry considered_texture,
			const vec2 target_position,
			const float target_rotation,
			const vec2 considered_size
		) const {
			auto points = make_sprite_points(target_position, considered_size, target_rotation);

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

			auto triangles = make_sprite_triangles(
				considered_texture,
				points,
				target_color, 
				in.flip 
			);

			if (effect == sprite_special_effect::COLOR_WAVE) {
				const auto left_col = rgba(hsv{ fmod(in.global_time_seconds / 2.f, 1.f), 1.0, 1.0 });
				const auto right_col = rgba(hsv{ fmod(in.global_time_seconds / 2.f + 0.3f, 1.f), 1.0, 1.0 });

				auto& t1 = triangles[0];
				auto& t2 = triangles[1];

				t1.vertices[0].color = t2.vertices[0].color = left_col;
				t2.vertices[1].color = right_col;
				t1.vertices[1].color = t2.vertices[2].color = right_col;
				t1.vertices[2].color = left_col;
			}

			in.output.push(triangles[0]);
			in.output.push(triangles[1]);
		}
	};
}