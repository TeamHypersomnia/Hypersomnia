#pragma once
#include "augs/math/vec2.h"
#include "augs/math/rects.h"

#include "augs/graphics/pixel.h"
#include "augs/graphics/vertex.h"
#include "game/assets/texture_id.h"
#include "transform_component.h"
#include "game/detail/camera_cone.h"
#include "game/enums/renderable_drawing_type.h"

struct state_for_drawing_camera;

namespace components {
	struct sprite {
		struct drawing_input : vertex_triangle_buffer_reference {
			using vertex_triangle_buffer_reference::vertex_triangle_buffer_reference;

			components::transform renderable_transform;
			camera_cone camera;

			enum class positioning_type : unsigned char {
				LEFT_TOP_CORNER,
				CENTER
			};

			positioning_type positioning = positioning_type::CENTER;

			augs::rgba colorize = augs::white;
			renderable_drawing_type drawing_type = renderable_drawing_type::NORMAL;
			float global_time_seconds = 0.f;

			void set_global_time_seconds(const float);
			void setup_from(const state_for_drawing_camera&);
		};

		assets::texture_id tex = assets::texture_id::INVALID;
		augs::rgba color;
		vec2 size;
		vec2 size_multiplier = vec2(1, 1);
		vec2 gui_bbox_expander;
		vec2 center_offset;
		float rotation_offset = 0.f;

		short flip_horizontally = false;
		short flip_vertically = false;

		enum class special_effect : unsigned char {
			NONE,
			COLOR_WAVE
		} effect = special_effect::NONE;
		unsigned char wave_speed_slowdown = 0u;

		unsigned short max_specular_blinks = 0;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(tex),
				CEREAL_NVP(color),
				CEREAL_NVP(size),
				CEREAL_NVP(size_multiplier),
				CEREAL_NVP(gui_bbox_expander),
				CEREAL_NVP(center_offset),
				CEREAL_NVP(rotation_offset),

				CEREAL_NVP(flip_horizontally),
				CEREAL_NVP(flip_vertically)
			);
		}

		static void make_rect(const vec2 pos, const vec2 size, const float rotation_degrees, std::array<vec2, 4>& out, const drawing_input::positioning_type positioning);

		vec2 get_size() const;

		void set(assets::texture_id, augs::rgba = augs::rgba());
		void update_size_from_texture_dimensions();

		void draw(const drawing_input&) const;

		std::vector<vec2> get_vertices() const;
		augs::rects::ltrb<float> get_aabb(const components::transform&, const drawing_input::positioning_type positioning = drawing_input::positioning_type::CENTER) const;
	};
}