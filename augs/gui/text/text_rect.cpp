#include "text_rect.h"
#include <algorithm>

namespace augs {
	namespace gui {
		namespace text {
			text_rect::text_rect(const rect& r, const fstr& _str) : rect(r), draft(_str) {
				clip = focusable = false;
			}

			void text_rect::draw_triangles(draw_info in) {
				draft.guarded_redraw();
				print.draw_text(in.v, draft.get_draft(), draft.get_str(), 0, *this);
			}

			void text_rect::set_text(const fstr& f) {
				if (draft.get_str().size() == f.size() && !memcmp(draft.get_str().data(), f.data(), f.size() * sizeof(formatted_char)))
					return;
				
				draft.str() = f;
			}

			void text_rect::center(rects::ltrb<float> r) {
				draft.guarded_redraw();
				auto bbox = draft.get_draft().get_bbox();

				rc = rects::xywh<float>(vec2i(r.w() / 2 - bbox.w / 2, r.h() / 2 - bbox.h / 2), rects::wh<float>(0, 0));
			}
		}
	}
}