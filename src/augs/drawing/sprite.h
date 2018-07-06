#pragma once
#include "augs/pad_bytes.h"

#include "augs/templates/value_with_flag.h"
#include "augs/math/vec2.h"
#include "augs/math/rects.h"

#include "augs/templates/introspect_declaration.h"
#include "augs/templates/traits/container_traits.h"

#include "augs/graphics/vertex.h"

#include "augs/drawing/make_sprite.h"
#include "augs/drawing/drawing_input_base.h"
#include "augs/drawing/drawing.h"

#include "augs/texture_atlas/atlas_entry.h"
#include "augs/build_settings/platform_defines.h"

struct intensity_vibration_input {
	// GEN INTROSPECTOR struct intensity_vibration_input
	real32 change_per_sec = 100.f;
	rgba_channel lower = 150;
	rgba_channel upper = 255;
	pad_bytes<2> pad;
	// END GEN INTROSPECTOR
};

namespace augs {
	enum class sprite_special_effect /* : unsigned char */ {
		// GEN INTROSPECTOR enum class augs::sprite_special_effect
		NONE = 0,
		COLOR_WAVE,
		CONTINUOUS_ROTATION
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
			const id_type image_id = {},
			const size_type size = size_type(),
			const rgba color = white
		) :
			image_id(image_id),
			size(size),
			color(color),
			neon_color(color)
		{}

		template <class M>
		sprite(
			const id_type image_id,
			const M& manager,
			const rgba color = white
		) {
			set(image_id, manager, color);
		}

		template <class M>
		void set(
			const id_type image_id,
			const M& manager,
			const rgba color = white
		) {
			this->image_id = image_id;
			this->size = manager.at(image_id).get_original_size();
			this->color = color;
			this->neon_color = color;
		}

		template <class T>
		void set(
			const id_type image_id,
			const basic_vec2<T> size,
			const rgba color = white
		) {
			this->image_id = image_id;
			this->size = size;
			this->color = color;
			this->neon_color = color;
		}
		
		void set_color(const rgba col) {
			color = col;
		}

		// GEN INTROSPECTOR struct augs::sprite class id_type
		id_type image_id;
		size_type size;
		rgba color = white;
		rgba neon_color = white;
		sprite_special_effect effect = sprite_special_effect::NONE;
		real32 effect_speed_multiplier = 1.f;
		augs::value_with_flag<intensity_vibration_input> neon_intensity_vibration;
		// END GEN INTROSPECTOR

		bool operator==(const sprite& b) const {
			return introspective_equal(*this, b);
		}

		vec2i get_size() const {
			return size;
		}

		ltrb get_aabb(const transformr transform) const {
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
				const auto& entry = manager.at(image_id);
				const auto& maybe_neon_map = entry.neon_map;

				if (maybe_neon_map.exists()) {
					draw(
						in,
						maybe_neon_map,
						transform_pos,
						final_rotation,
						vec2(maybe_neon_map.get_original_size())
						/ entry.diffuse.get_original_size() * drawn_size,
						neon_color
					);
				}
			}
			else {
				draw(
					in,
					manager.at(image_id).diffuse,
					transform_pos,
					final_rotation,
					drawn_size,
					color
				);
			}
		}
		
	private:
		FORCE_INLINE void draw(
			const drawing_input in,
			const atlas_entry considered_texture,
			const vec2 target_position,
			float target_rotation,
			const vec2i considered_size,
			rgba target_color
		) const {
			if (effect == sprite_special_effect::CONTINUOUS_ROTATION) {
				target_rotation += std::fmod(in.global_time_seconds * effect_speed_multiplier * 360.f, 360.f);
			}

			if (in.colorize != white) {
				target_color *= in.colorize;
			}

			const auto points = make_sprite_points(target_position, considered_size, target_rotation);

			auto triangles = make_sprite_triangles(
				considered_texture,
				points,
				target_color, 
				in.flip 
			);

			if (effect == sprite_special_effect::COLOR_WAVE) {
				auto left_col = rgba(hsv{ std::fmod(in.global_time_seconds * effect_speed_multiplier / 2.f, 1.f), 1.0, 1.0 });
				auto right_col = rgba(hsv{ std::fmod(in.global_time_seconds * effect_speed_multiplier / 2.f / 2.f + 0.3f, 1.f), 1.0, 1.0 });

				left_col.avoid_dark_blue_for_color_wave();
				right_col.avoid_dark_blue_for_color_wave();

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