#pragma once
#include "text/drafter.h"
#include "text/printer.h"
#include "text/draft_interface.h"
#include "rect_world.h"

namespace augs {
	namespace gui {
		struct text_drawer {
			typedef std::vector<vertex_triangle> buf;

			text::draft_redrawer draft;
			text::printer print;
			vec2i pos;
			
			vec2i get_bbox();

			void set_text(const text::fstr&);

			void draw_stroke(buf&, rgba col = black);
			void draw(buf&);
			void draw(rect::draw_info);

			void center(rects::ltrb<float>);
			void bottom_right(rects::ltrb<float>);
			void above_left_to_right(vec2i pos);
			void below_left_to_right(vec2i pos);
		};
	}
}