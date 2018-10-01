#pragma once
#include <optional>

#include "augs/misc/timing/timer.h"
#include "augs/drawing/drawing.h"
#include "augs/gui/formatted_string.h"
#include "augs/misc/enum/enum_boolset.h"

namespace augs {
	enum class ralign {
		CX, CY, RT, RB, LB, COUNT
	};

	using center_flags = augs::enum_boolset<ralign>;

	namespace gui {
		namespace text {
			struct caret_info;
			struct drafter;

			struct caret_blinker {
				bool blink = true;
				bool caret_visible = true;
				unsigned interval_ms = 250;

				timer timing;

				void update();
				void reset();
			};

			struct printer {
				caret_blinker blink;
				rgba selected_text_color = white;

				unsigned caret_width = 1;

				bool active = false;
				bool align_caret_height = true;
				bool highlight_current_line = false;
				bool highlight_during_selection = true;

				rgba caret_col = white;
				rgba highlight_col = rgba(15, 15, 15, 255);
				rgba selection_bg_col = rgba(128, 255, 255, 120);
				rgba selection_inactive_bg_col = rgba(128, 255, 255, 40);

				void draw_text(
					const drawer out,
					const vec2i pos,
					const drafter&,
					const ltrbi clipper = ltrbi()
				) const;

				void draw_text(
					const drawer_with_default out,
					const vec2i pos,
					const drafter&,
					const caret_info caret,
					const ltrbi clipper = ltrbi()
				) const;
			};

			vec2i get_text_bbox(
				const formatted_string& str, 
				const unsigned wrapping_width = 0,
				const bool use_kerning = false
			);

			vec2i print(
				const drawer out,
				const vec2i pos,
				const formatted_string& str,
				const unsigned wrapping_width = 0,
				const ltrbi clipper = ltrbi(),
				const bool use_kerning = false
			);

			vec2i print_stroked(
				const drawer out,
				const vec2i pos,
				const formatted_string& str,
				const center_flags = {},
				const rgba stroke_color = black,
				const unsigned wrapping_width = 0,
				const ltrbi clipper = ltrbi(),
				const bool use_kerning = false
			);

			vec2i print(
				const drawer_with_default out,
				const vec2i pos,
				const formatted_string& str,
				const caret_info caret,
				const unsigned wrapping_width = 0,
				const ltrbi clipper = ltrbi(),
				const bool use_kerning = false
			);

			vec2i print_stroked(
				const drawer_with_default out,
				const vec2i pos,
				const formatted_string& str,
				const caret_info caret,
				const rgba stroke_color = black,
				const unsigned wrapping_width = 0,
				const ltrbi clipper = ltrbi(),
				const bool use_kerning = false
			);
		}
	}
}