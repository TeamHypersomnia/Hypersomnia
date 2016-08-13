#pragma once
#include "augs/math/vec2.h"
#include "augs/math/rects.h"

#include "augs/graphics/pixel.h"
#include "augs/graphics/vertex.h"
#include "game/assets/texture_id.h"
#include "transform_component.h"

struct state_for_drawing_camera;

namespace components {
	struct sprite {
		struct drawing_input : vertex_triangle_buffer_reference {
			using vertex_triangle_buffer_reference::vertex_triangle_buffer_reference;

			components::transform renderable_transform;
			components::transform camera_transform;
			vec2 visible_world_area;

			bool screen_space_mode = false;
			bool position_is_left_top_corner = false;

			augs::rgba colorize = augs::white;

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

		static void make_rect(vec2 pos, vec2 size, float rotation_degrees, std::array<vec2, 4>& out, bool pos_at_center);

		vec2 get_size() const;

		void set(assets::texture_id, augs::rgba = augs::rgba());
		void update_size_from_texture_dimensions();

		void draw(const drawing_input&) const;

		std::vector<vec2> get_vertices() const;
		augs::rects::ltrb<float> get_aabb(components::transform, bool screen_space_mode = false) const;
	};
}