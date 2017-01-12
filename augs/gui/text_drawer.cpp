#include "text_drawer.h"
#include <algorithm>
#include "augs/gui/text/caret.h"

namespace augs {
	namespace gui {
		void text_drawer::draw_stroke(buf& v, const rgba col, const text::caret_info* in) {
			draft.guarded_redraw();

			auto coloured_str = draft.get_str();
			
			for (auto& c : coloured_str) {
				c.set(c.font_used, col);
			}

			print.draw_text(v, draft.get_draft(), coloured_str, in, pos + vec2(-1, 0));
			print.draw_text(v, draft.get_draft(), coloured_str, in, pos + vec2(1, 0));
			print.draw_text(v, draft.get_draft(), coloured_str, in, pos + vec2(0, -1));
			print.draw_text(v, draft.get_draft(), coloured_str, in, pos + vec2(0, 1));
		}
		
		void text_drawer::draw(buf& v, const text::caret_info* in) {
			draft.guarded_redraw();
			print.draw_text(v, draft.get_draft(), draft.get_str(), in, pos);
		}

		void text_drawer::draw(draw_info in) {
			draw(in.v);
		}

		void text_drawer::set_text(const text::fstr& f) {
			if (draft.get_str().size() == f.size() && !memcmp(draft.get_str().data(), f.data(), f.size() * sizeof(text::formatted_char))) {
				return;
			}

			draft.str() = f;
		}

		vec2i text_drawer::get_bbox() {
			draft.guarded_redraw();
			return draft.get_draft().get_bbox();
		}

		void text_drawer::center(const ltrbi r) {
			const auto bbox = get_bbox();
			pos = vec2i(r.l + r.w() / 2 - bbox.x / 2, r.t + r.h() / 2 - bbox.y / 2);
		}

		void text_drawer::bottom_right(const ltrbi r) {
			const auto bbox = get_bbox();
			pos = vec2i(r.r - bbox.x, r.b - bbox.y);
		}

		void text_drawer::above_left_to_right(const vec2i p) {
			pos = vec2i(p.x, p.y - get_bbox().y);
		}

		void text_drawer::below_left_to_right(const vec2i p) {
			pos = vec2i(p.x, p.y + get_bbox().y);
		}
	}
}