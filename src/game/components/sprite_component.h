#pragma once
#include "augs/math/vec2.h"
#include "augs/math/rects.h"

#include "augs/graphics/rgba.h"
#include "augs/graphics/vertex.h"
#include "game/assets/game_image_id.h"
#include "transform_component.h"
#include "game/detail/basic_renderable_drawing_input.h"
#include "game/enums/renderable_drawing_type.h"

#include "augs/graphics/drawers.h"
#include "augs/pad_bytes.h"

namespace components {
	struct sprite {
		struct drawing_input : basic_renderable_drawing_input {
			using basic_renderable_drawing_input::basic_renderable_drawing_input;

			double global_time_seconds = 0.0;
			void set_global_time_seconds(const double);
		};

		enum class special_effect : unsigned char {
			NONE,
			COLOR_WAVE
		};

		// GEN INTROSPECTOR struct components::sprite
		assets::game_image_id tex = assets::game_image_id::INVALID;
		rgba color;
		vec2 overridden_size = vec2(-1.f, -1.f);
		vec2 center_offset;
		float rotation_offset = 0.f;

		bool flip_horizontally = false;
		bool flip_vertically = false;
		
		special_effect effect = special_effect::NONE;
		pad_bytes<1> pad;

		unsigned max_specular_blinks = 0;
		// END GEN INTROSPECTOR

		template <class T>
		vec2 get_size(const T& metas) const {
			if (overridden_size.x > 0.f) {
				return overridden_size;
			}
			else {
				return metas[tex].get_size();
			}
		}

		void set(
			const assets::game_image_id id,
			const vec2 size,
			const rgba color = rgba()
		);

		void set(
			const assets::game_image_id id,
			const rgba color = rgba()
		);

		void draw(const drawing_input) const;
		
		void draw_from_lt(drawing_input) const;

		void draw(
			const drawing_input,
			const augs::texture_atlas_entry considered_texture,
			const vec2i target_position,
			const float target_rotation,
			const vec2 considered_size
		) const;

		template <class T>
		ltrb get_aabb(
			const T& metas,
			const components::transform transform
		) const {
			return augs::get_aabb(
				augs::make_sprite_points(
					transform.pos, 
					get_size(metas),
					transform.rotation + rotation_offset
				)
			);
		}
	};
}