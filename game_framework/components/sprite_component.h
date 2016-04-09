#pragma once
#include "math/vec2.h"
#include "math/rects.h"

#include "graphics/pixel.h"
#include "../assets/texture.h"
#include "transform_component.h"

namespace shared {
	struct state_for_drawing_renderable;
}

namespace components {
	struct sprite {
		static void make_rect(vec2 pos, vec2 size, float rotation_degrees, vec2 out[4], bool pos_at_center);

		assets::texture_id tex = assets::texture_id::INVALID_TEXTURE;
		augs::rgba color;
		vec2 size;
		vec2 gui_bbox_expander;
		vec2 center_offset;
		float rotation_offset = 0.f;

		bool flip_horizontally = false;
		bool flip_vertically = false;

		void set(assets::texture_id, augs::rgba = augs::rgba());
		void update_size();

		void draw(const shared::state_for_drawing_renderable&) const;

		std::vector<vec2> get_vertices() const;
		augs::rects::ltrb<float> get_aabb(components::transform, bool screen_space_mode = false) const;
	};
}