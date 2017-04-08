#pragma once
#include "augs/math/vec2.h"
#include "augs/math/rects.h"

#include "augs/graphics/pixel.h"
#include "augs/graphics/vertex.h"
#include "game/assets/game_image_id.h"
#include "transform_component.h"
#include "game/detail/basic_renderable_drawing_input.h"
#include "game/enums/renderable_drawing_type.h"

#include "augs/padding_byte.h"

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
		assets::game_image_id tex = assets::game_image_id::COUNT;
		rgba color;
		vec2 size;
		vec2 center_offset;
		float rotation_offset = 0.f;

		short flip_horizontally = false;
		short flip_vertically = false;
		
		special_effect effect = special_effect::NONE;
		padding_byte pad;

		unsigned short max_specular_blinks = 0;
		// END GEN INTROSPECTOR

		vec2 get_size() const;

		void set(
			const assets::game_image_id id,
			const vec2 size,
			const rgba color = rgba()
		);

		void set(
			const assets::game_image_id id,
			const cosmos& manager,
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

		std::vector<vec2> get_vertices() const;
		
		ltrb get_aabb(
			const components::transform
		) const;
	};
}