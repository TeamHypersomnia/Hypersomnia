#pragma once
#include "../rect.h"
#include "printer.h"
#include "draft_interface.h"
namespace augs {
	namespace gui {
		namespace text {
			struct text_rect : public rect {
				draft_redrawer draft;
				text::printer print;

				void set_text(const fstr&);

				text_rect(const rect& = rect(), const fstr& = fstr());

				virtual void draw_triangles(draw_info) override;

				void center(rects::ltrb<float>);
			};
		}
	}
}