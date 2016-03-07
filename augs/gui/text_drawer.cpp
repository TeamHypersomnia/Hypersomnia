#include "text_drawer.h"
#include <algorithm>

namespace augs {
	namespace gui {
		void text_drawer::draw(rect::draw_info in) {
			draft.guarded_redraw();
			print.draw_text(in.v, draft.get_draft(), draft.get_str(), nullptr, pos, nullptr);
		}

		void text_drawer::set_text(const text::fstr& f) {
			if (draft.get_str().size() == f.size() && !memcmp(draft.get_str().data(), f.data(), f.size() * sizeof(text::formatted_char)))
				return;

			draft.str() = f;
		}

		void text_drawer::center(rects::ltrb<float> r) {
			draft.guarded_redraw();
			auto bbox = draft.get_draft().get_bbox();

			pos = vec2i(r.l + r.w() / 2 - bbox.w / 2, r.t + r.h() / 2 - bbox.h / 2);
		}

		void text_drawer::bottom_right(rects::ltrb<float> r) {
			draft.guarded_redraw();
			auto bbox = draft.get_draft().get_bbox();

			pos = vec2i(r.r - bbox.w, r.b - bbox.h);
		}
	}
}