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

		rects::wh<float> rect::get_content_size() const {

		}

		void rect::draw_stretched_texture(draw_info in, const material& mat) const {
			draw_clipped_rectangle(mat, get_rect_absolute(), parent, in.v).good();
			// rc_clipped = draw_clipped_rectangle(mat, rc_clipped, parent, in.v);
		}

		void rect::draw_centered_texture(draw_info in, const material& mat, vec2i offset) const {
			auto absolute_centered = get_rect_absolute();
			auto tex_size = (*mat.tex).get_size();
			absolute_centered.l += absolute_centered.w() / 2 - float(tex_size.x) / 2;
			absolute_centered.t += absolute_centered.h() / 2 - float(tex_size.y) / 2;
			absolute_centered.l = int(absolute_centered.l) + offset.x;
			absolute_centered.t = int(absolute_centered.t) + offset.y;
			absolute_centered.w(tex_size.x);
			absolute_centered.h(tex_size.y);

			draw_clipped_rectangle(mat, absolute_centered, parent, in.v).good();
			// rc_clipped = draw_clipped_rectangle(mat, rc_clipped, parent, in.v);
		}

		void rect::draw_rectangle_stylesheeted(draw_info in, const stylesheet& styles) const {
			auto st = styles.get_style();

			if (st.color.active || st.background_image.active)
				draw_stretched_texture(in, material(st));

			if (st.border.active) st.border.value.draw(in.v, *this);
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
