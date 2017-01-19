#pragma once
// got to revise gui systems in terms of rectangle update'ing
#include "augs/misc/timer.h"
#include "augs/gui/material.h"

namespace augs {
	namespace gui {
		namespace text {
			struct caret_info;
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
					const caret_info* const caret,
					const vec2i pos,
					const ltrbi clipper = ltrbi()
				) const;
			};

			extern vec2i get_text_bbox(
				const fstr& str, 
				const unsigned wrapping_width
			);

			extern vec2 quick_print(
				std::vector<augs::vertex_triangle>& v,
				const fstr& str,
				const vec2i pos,
				const unsigned wrapping_width = 0,
				const ltrbi clipper = ltrbi()
			);

			extern vec2 quick_print_format(
				std::vector<augs::vertex_triangle>& v,
				const std::wstring& wstr,
				const style style,
				const vec2i pos,
				const unsigned wrapping_width = 0,
				const ltrbi clipper = ltrbi()
			);
		}
	}
}