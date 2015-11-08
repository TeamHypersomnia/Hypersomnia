#include "text_rect.h"

namespace augs {
	namespace graphics {
		using namespace augs::texture_baker;
		namespace gui {
			namespace text {
				text_rect::text_rect(const rect& r, const fstr& _str) : rect(r), draft(_str) {
					clip = focusable = false;
				}
					
				void text_rect::draw(draw_info in, rect& subject) {
					draft.guarded_redraw();
					print.draw_text(in.v, draft.get_draft(), draft.get_str(), 0, subject);
				}

				void text_rect::draw_proc(draw_info in) {
					draw(in, *this);
				}

				void text_rect::center(rects::ltrb<float> r) {
					draft.guarded_redraw();
					auto bbox = draft.get_draft().get_bbox();

					rc = rects::xywh<float>(vec2i(r.w()/2 - bbox.w/2, r.h()/2 - bbox.h/2), rects::wh<float>(0, 0));
				}

			}
		}
	}
}