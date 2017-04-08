#include "rect.h"
#include "stylesheet.h"
#include "game/assets/assets_manager.h"

namespace augs {
	namespace gui {
		rect_node_data::rect_node_data(const xywh& rc) : rc(rc) {
			set_default_flags();
		}

		rect_node_data::rect_node_data(const assets::game_image_id& id) {
			const auto& manager = get_assets_manager();

			set_default_flags();
			rc.set_size(manager[id].get_size());
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
		//	return vec2(rc).can_contain(content_size, scroll);
		//}
		//
		//void rect_composite::clamp_scroll_to_right_down_corner() {

		//void clamp_offset_to_right_down_corner_of(const wh& bigger, vec2t<T>& offset) const {
		//	offset.x = std::min(offset.x, T(bigger.w - w));
		//	offset.x = std::max(offset.x, 0.f);
		//	offset.y = std::min(offset.y, T(bigger.h - h));
		//	offset.y = std::max(offset.y, 0.f);
		//}

		//	vec2(rc).clamp_offset_to_right_down_corner_of(content_size, scroll);
		//}
		//
		//ltrb rect_composite::get_local_clipper() const {
		//	return ltrb(vec2(rc)) + scroll;
		//}
		//
		//ltrb rect_leaf::get_absolute_rect() const {
		//	return xywh(absolute_xy.x, absolute_xy.y, rc.w(), rc.h());
		//}
	}
}
