#pragma once
// got to revise gui systems in terms of rectangle update'ing
#include "augs/misc/timer.h"
#include "augs/gui/material.h"

namespace augs {
	namespace gui {
		namespace text {
			struct caret_info;
			class ui;
			struct drafter;
			struct printer {
				/* defines how the caret should blink and whether should blink at all */
				struct blinker {
					bool blink, caret_visible;
					int interval_ms;

					//static void regular_blink(blinker&, quad& caret);
					//void (*blink_func)(blinker&, quad&);

					timer timer;
					blinker();
					void update();
					void reset();
				};

				blinker blink;
				rgba selected_text_color;

				unsigned caret_width;

				bool active,
					align_caret_height, /* whether caret should be always of line height */
					highlight_current_line,
					highlight_during_selection;

				material caret_mat,
					highlight_mat,
					selection_bg_mat,
					selection_inactive_bg_mat; /* material for line highlighting */

				printer();

				void draw_text(
					std::vector<augs::vertex_triangle>& out,
					const drafter&,
					const fstr& colors,
					/* if caret is 0, draw no caret */
					const caret_info* caret,
					vec2i pos,
					rects::ltrb<float> clipper = rects::ltrb<float>()) const;
			};

			/*
			parent shifts position and clips the text
			wrapping_width = 0 means no wrapping
			 parent = 0 means no clipping/shifting
			returns text's bounding box (without clipping)
			*/
			extern vec2i get_text_bbox(const std::basic_string<formatted_char>& str, unsigned wrapping_width);

			extern rects::wh<float> quick_print(std::vector<augs::vertex_triangle>& v,
				const fstr& str,
				vec2i pos,
				unsigned wrapping_width = 0,
				rects::ltrb<float> clipper = rects::ltrb<float>());

			extern rects::wh<float> quick_print_format(std::vector<augs::vertex_triangle>& v,
				const std::wstring& wstr,
				style style,
				vec2i pos,
				unsigned wrapping_width = 0,
				rects::ltrb<float> clipper = rects::ltrb<float>());
		}
	}
}