#include "rect.h"
#include "stylesheet.h"
#include "augs/window_framework/window.h"

#include <algorithm>
#include "augs/ensure.h"
#include "rect_world.h"

#undef max
#undef min

namespace augs {
	namespace gui {
		void rect_leaf::set_default_flags() {
			unset_flag(flag::DISABLE_HOVERING);
			set_flag(flag::ENABLE_DRAWING);
			unset_flag(flag::FETCH_WHEEL);
			unset_flag(flag::PRESERVE_FOCUS);
			set_flag(flag::FOCUSABLE);
			set_flag(flag::ENABLE_DRAWING_OF_CHILDREN);
			set_flag(flag::SNAP_SCROLL_TO_CONTENT_SIZE);
			set_flag(flag::SCROLLABLE);
			set_flag(flag::CLIP);
		}

		bool rect_leaf::get_flag(const flag f) const {
			return flags.test(static_cast<size_t>(f));
		}

		bool rect_leaf::set_flag(const flag f) {
			flags.set(static_cast<size_t>(f));
		}

		bool rect_leaf::unset_flag(const flag f) {
			flags.set(static_cast<size_t>(f), false);
		}

		rect_leaf::rect_leaf(rects::xywh<float> rc) : rc(rc) {
			set_default_flags();
		}

		rect_leaf::rect_leaf(assets::texture_id id) {
			set_default_flags();
			rc.set_size((*id).get_size());
		 }

		bool rect_leaf::is_being_dragged(const gui::rect_world& g) const {
			return g.rect_held_by_lmb == this_id && g.held_rect_is_dragged;
		}

		/* handle focus and passing scroll to parents */

		bool rect_composite::is_scroll_clamped_to_right_down_corner() const {
			return rects::wh<float>(rc).can_contain(content_size, scroll);
		}

		void rect_composite::clamp_scroll_to_right_down_corner() {
			rects::wh<float>(rc).clamp_offset_to_right_down_corner_of(content_size, scroll);
		}

		vec2 rect_leaf::get_scroll() const {
			return{ 0.f, 0.f };
		}

		void rect_leaf::set_scroll(const vec2) {
		
		}

		vec2 rect_composite::get_scroll() const {
			return scroll;
		}

		void rect_composite::set_scroll(const vec2 v) {
			scroll = v;
		}

		rects::ltrb<float> rect_leaf::get_clipping_rect() const {
			return{ 0.f, 0.f, 0.f, 0.f };
		}

		const rects::ltrb<float>& rect_leaf::get_clipped_rect() const {
			return rc_clipped;
		}

		rects::ltrb<float> rect_composite::get_local_clipper() const {
			return rects::ltrb<float>(rects::wh<float>(rc)) + scroll;
		}

		rects::ltrb<float> rect_composite::get_clipping_rect() const {
			return clipping_rect;
		}

		rects::ltrb<float> rect_leaf::get_rect_absolute() const {
			return rects::xywh<float>(absolute_xy.x, absolute_xy.y, rc.w(), rc.h());
		}

		const vec2i& rect_leaf::get_absolute_xy() const {
			return absolute_xy;
		}
	}
}
