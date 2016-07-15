#pragma once
#include "rect.h"
#include "stylesheet.h"
#include "window_framework/window.h"

#include <algorithm>
#include <functional>
#include "ensure.h"
#include "rect_world.h"

#undef max
#undef min
namespace augs {
	namespace gui {
		draw_info::draw_info(const rect_world& owner, vertex_triangle_buffer& v) : owner(owner), v(v) {}
		raw_event_info::raw_event_info(rect_world& owner, unsigned msg) : owner(owner), msg(msg), mouse_fetched(false), scroll_fetched(false) {}
		event_info::event_info(rect_world& owner, gui_event msg) : owner(owner), msg(msg) {}

		event_info::operator gui_event() {
			return msg;
		}

		event_info& event_info::operator=(gui_event m) {
			msg = m;
			return *this;
		}

		rect::rect(rects::xywh<float> rc) : rc(rc) {}
		rect::rect(assets::texture_id id) {
			rc.set_size((*id).get_size());
		 }

		/* handle focus and passing scroll to parents */

		bool rect::is_scroll_clamped_to_right_down_corner() {
			return rects::wh<float>(rc).can_contain(content_size, scroll);
		}

		void rect::clamp_scroll_to_right_down_corner() {
			rects::wh<float>(rc).clamp_offset_to_right_down_corner_of(content_size, scroll);
		}
		
		const rects::ltrb<float>& rect::get_clipped_rect() const {
			return rc_clipped;
		}

		rects::ltrb<float> rect::get_local_clipper() const {
			return rects::ltrb<float>(rects::wh<float>(rc)) + scroll;
		}

		rects::ltrb<float> rect::get_clipping_rect() const {
			return clipping_rect;
		}

		rects::ltrb<float> rect::get_rect_absolute() const {
			return rects::xywh<float>(absolute_xy.x, absolute_xy.y, rc.w(), rc.h());
		}

		const vec2i& rect::get_absolute_xy() const {
			return absolute_xy;
		}
	}
}
