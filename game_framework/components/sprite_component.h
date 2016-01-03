#pragma once
#include "math/vec2.h"

#include "graphics/pixel.h"
#include "../assets/texture.h"

namespace shared {
	class drawing_state;
}

namespace components {
	class sprite {
	public:
		static void make_rect(vec2 pos, vec2 size, float rotation_degrees, vec2 out[4]);

		assets::texture_id tex = assets::BLANK;
		augs::pixel_32 color;
		vec2 size;
		float rotation_offset;

		sprite(assets::texture_id = assets::BLANK, augs::pixel_32 = augs::pixel_32());

		void set(assets::texture_id, augs::pixel_32);
		void update_size();

		std::vector<vec2> get_vertices();
		void draw(shared::drawing_state&);
	};
}