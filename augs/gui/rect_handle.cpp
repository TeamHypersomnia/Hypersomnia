#include "rect_handle.h"
#include "rect.h"

typedef augs::basic_pool<augs::gui::rect> B;
typedef augs::gui::rect R;

namespace augs {
	template <bool C>
	template <class>
	basic_handle<C, B, R>::operator basic_handle<true, B, R>() const {
		return basic_handle<true, B, R>(owner, raw_id);
	}

	template <bool C>
	template <class>
	void basic_handle<C, B, R>::calculate_clipped_rectangle_layout(gui::content_size_behaviour behaviour) const {
		/* init; later to be processed absolute and clipped with local rc */
		auto& self = get();
		
		self.rc_clipped = self.rc;
		self.absolute_xy = vec2i(self.rc.l, self.rc.t);

		/* if we have parent */
		if (get_parent().alive()) {
			auto& p = get_parent().get();

			/* we have to save our global coordinates in absolute_xy */
			self.absolute_xy = p.absolute_xy + vec2i(self.rc.l, self.rc.t) - vec2i(int(p.scroll.x), int(p.scroll.y));
			self.rc_clipped = rects::xywh<float>(self.absolute_xy.x, self.absolute_xy.y, self.rc.w(), self.rc.h());

			/* and we have to clip by first clipping parent's rc_clipped */
			//auto* clipping = get_clipping_parent(); 
			//if(clipping) 
			self.rc_clipped.clip_by(p.clipping_rect);

			self.clipping_rect = p.clipping_rect;
		}

		if (self.clip)
			self.clipping_rect.clip_by(self.rc_clipped);

		/* update content size */
		self.content_size = behaviour(*this);

		/* align scroll only to be positive and not to exceed content size */
		if (self.snap_scroll_to_content_size)
			clamp_scroll_to_right_down_corner();

		/* do the same for every child */
		auto children_all = get_pool()[children];
		for (size_t i = 0; i < children_all.size(); ++i) {
			children_all[i].get().parent = this;
			//if (children_all[i]->enable_drawing)
			children_all[i].calculate_clipped_rectangle_layout(behaviour);
		}
	}

	template <bool C>
	basic_handle<C, B, R> basic_handle<C, B, R>::get_parent() const {
		return get_pool()[get().parent];
	}
}

// explicit instantiation
template class augs::basic_handle<true, B, R>;
template class augs::basic_handle<false, B, R>;