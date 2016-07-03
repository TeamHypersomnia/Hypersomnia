#pragma once
#include "math/vec2.h"
#include "math/rects.h"

#include "graphics/pixel.h"
#include "graphics/vertex.h"
#include "game/assets/texture_id.h"
#include "transform_component.h"

struct state_for_drawing_camera;

namespace components {
	struct sprite {
		struct drawing_input : vertex_triangle_buffer_reference {
			using vertex_triangle_buffer_reference::vertex_triangle_buffer_reference;

			vec2 visible_world_area;

			components::transform renderable_transform;
			components::transform camera_transform;

			bool screen_space_mode = false;
			bool position_is_left_top_corner = false;

			augs::rgba colorize = augs::white;

			void setup_from(const state_for_drawing_camera&);
		};

		static void make_rect(vec2 pos, vec2 size, float rotation_degrees, vec2 out[4], bool pos_at_center);

		assets::texture_id tex = assets::texture_id::INVALID_TEXTURE;
		augs::rgba color;
		vec2 size, size_multiplier = vec2(1, 1);
		vec2 gui_bbox_expander;
		vec2 center_offset;
		float rotation_offset = 0.f;

		bool flip_horizontally = false;
		bool flip_vertically = false;

		vec2 get_size() const;

		void set(assets::texture_id, augs::rgba = augs::rgba());
		void update_size_from_texture_dimensions();

		void draw(const drawing_input&) const;

		std::vector<vec2> get_vertices() const;
		augs::rects::ltrb<float> get_aabb(components::transform, bool screen_space_mode = false) const;
	};
}