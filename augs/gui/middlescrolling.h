#pragma once
#include "material.h"
#include "math/rects.h"
#include "rect.h"
#include "misc/delta.h"
#include "window_framework/event.h"

namespace augs {
	namespace gui {
		struct middlescrolling {
			material mat;
			rects::wh<float> size = rects::wh<float>(25, 25);
			vec2i pos;
			rect_id subject;
			float speed_mult = 1.f;

			void perform_logic_step(rect_pool&, fixed_delta, window::event::state state);
			bool handle_new_raw_state(rect_pool&, window::event::state);
			void draw_triangles(const rect_pool&, rect::draw_info) const;
		};
	}
}