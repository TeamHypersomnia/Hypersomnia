#pragma once
#include "text/drafter.h"
#include "text/printer.h"
#include "text/draft_interface.h"
#include "gui_world.h"

namespace augs {
	namespace gui {
		struct text_drawer {
			text::draft_redrawer draft;
			text::printer print;
			vec2i pos;

			void set_text(const text::fstr&);

			void draw(rect::draw_info);

			void center(rects::ltrb<float>);
			void bottom_right(rects::ltrb<float>);
		};
	}
}