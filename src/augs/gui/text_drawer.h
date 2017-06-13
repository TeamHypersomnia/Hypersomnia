#pragma once
#include "text/drafter.h"
#include "text/printer.h"
#include "text/draft_redrawer.h"
#include "rect_world.h"

namespace augs {
	namespace gui {
		namespace text {
			struct caret_info;
		}

		struct text_drawer {
			typedef std::vector<vertex_triangle> buf;

			text::draft_redrawer draft;
			text::printer print;
			vec2i pos;
			
			vec2i get_bbox();

			void set_text(const text::formatted_string&);

			void draw_stroke(buf&, const rgba col = black, const text::caret_info* caret = nullptr);
			void draw(buf&, const text::caret_info* in = nullptr);
			void draw(draw_info);

			void center(const ltrbi);
			void bottom_right(const ltrbi);
			void above_left_to_right(const vec2i pos);
			void below_left_to_right(const vec2i pos);
		};
	}
}