#pragma once
#include <unordered_map>

#include "augs/math/rects.h"

namespace augs {
	namespace gui {
		template <class gui_element_polymorphic_id>
		class rect_tree_entry {
			ltrb rc;
			gui_element_polymorphic_id parent;
			vec2 absolute_position;
			ltrb absolute_clipped_rect;
			ltrb absolute_clipping_rect;
		public:
			rect_tree_entry(const ltrb& rc) : rc(rc) {}

			void set_parent(const gui_element_polymorphic_id& id) {
				parent = id;
			}

			void set_absolute_clipping_rect(const ltrb& r) {
				absolute_clipping_rect = r;
			}

			void set_absolute_clipped_rect(const ltrb& r) {
				absolute_clipped_rect = r;
			}

			void set_absolute_pos(const vec2 v) {
				absolute_position = v;
			}

			gui_element_polymorphic_id get_parent() const {
				return parent;
			}

			ltrb get_absolute_rect() const {
				return rects::xywh<float>(absolute_position.x, absolute_position.y, rc.w(), rc.h());
			}

			ltrb get_absolute_clipping_rect() const {
				return absolute_clipping_rect;
			}

			ltrb get_absolute_clipped_rect() const {
				return absolute_clipped_rect;
			}

			vec2 get_absolute_pos() const {
				return absolute_position;
			}
		};

		template <class gui_element_polymorphic_id>
		using rect_tree = std::unordered_map<gui_element_polymorphic_id, rect_tree_entry<gui_element_polymorphic_id>>;
	}
}