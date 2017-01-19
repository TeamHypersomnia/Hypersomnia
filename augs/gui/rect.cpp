#include "rect.h"
#include "stylesheet.h"
#include "game/resources/manager.h"

namespace augs {
	namespace gui {
		rect_node_data::rect_node_data(const xywh& rc) : rc(rc) {
			set_default_flags();
		}

		rect_node_data::rect_node_data(const assets::texture_id& id) {
			set_default_flags();
			rc.set_size((*id).get_size());
		}

		void rect_node_data::set_default_flags() {
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

		bool rect_node_data::get_flag(const flag f) const {
			return flags.test(f);
		}

		void rect_node_data::set_flag(const flag f, const bool value) {
			flags.set(f, value);
		}

		void rect_node_data::unset_flag(const flag f) {
			flags.set(f, false);
		}

		vec2 rect_node_data::get_scroll() const {
			return vec2();
		}

		void rect_node_data::set_scroll(const vec2) {

		}

		///* handle focus and passing scroll to parents */
		//
		//bool rect_composite::is_scroll_clamped_to_right_down_corner() const {
		//	return rects::wh<float>(rc).can_contain(content_size, scroll);
		//}
		//
		//void rect_composite::clamp_scroll_to_right_down_corner() {
		//	rects::wh<float>(rc).clamp_offset_to_right_down_corner_of(content_size, scroll);
		//}
		//
		//ltrb rect_composite::get_local_clipper() const {
		//	return ltrb(rects::wh<float>(rc)) + scroll;
		//}
		//
		//ltrb rect_leaf::get_absolute_rect() const {
		//	return xywh(absolute_xy.x, absolute_xy.y, rc.w(), rc.h());
		//}
	}
}
