#pragma once
#include "augs/math/vec2.h"
#include "augs/math/rects.h"

#include "augs/graphics/pixel.h"
#include "augs/graphics/vertex.h"
#include "game/assets/texture_id.h"
#include "transform_component.h"
#include "game/detail/basic_renderable_drawing_input.h"
#include "game/enums/renderable_drawing_type.h"

namespace augs {
	class texture_with_image;
}

namespace components {
	struct sprite {
		struct drawing_input : basic_renderable_drawing_input {
			using basic_renderable_drawing_input::basic_renderable_drawing_input;

			float global_time_seconds = 0.f;
			void set_global_time_seconds(const float);
		};

		assets::texture_id tex = assets::texture_id::INVALID;
		rgba color;
		vec2 size;
		vec2 size_multiplier = vec2(1, 1);
		vec2 center_offset;
		float rotation_offset = 0.f;

		short flip_horizontally = false;
		short flip_vertically = false;

		enum class special_effect : unsigned char {
			NONE,
			COLOR_WAVE
		} effect = special_effect::NONE;
		bool has_neon_map = false;

		unsigned short max_specular_blinks = 0;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(tex),
				CEREAL_NVP(color),
				CEREAL_NVP(size),
				CEREAL_NVP(size_multiplier),
				CEREAL_NVP(bbox_expander),
				CEREAL_NVP(center_offset),
				CEREAL_NVP(rotation_offset),

				CEREAL_NVP(flip_horizontally),
				CEREAL_NVP(flip_vertically)
			);
		}

		vec2 get_size() const;

		void set(assets::texture_id, rgba = rgba());
		void update_size_from_texture_dimensions();

		void draw(const drawing_input&) const;

		void draw(
			const drawing_input&,
			const augs::texture_atlas_entry& considered_texture,
			const vec2i target_position,
			const float target_rotation,
			const vec2 considered_size
		) const;

		std::vector<vec2> get_vertices() const;
		
		ltrb get_aabb(
			const components::transform&, 
			const renderable_positioning_type positioning = renderable_positioning_type::CENTER
		) const;
	};
}