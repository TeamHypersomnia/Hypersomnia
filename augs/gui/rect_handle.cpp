#include "rect_handle.h"
#include "rect.h"
#include "gui_event.h"

#include "stylesheet.h"
#include "window_framework/window.h"

#include <algorithm>
#include <functional>
#include "ensure.h"
#include "rect_world.h"

#undef max
#undef min

using namespace augs::gui;

typedef augs::basic_pool<augs::gui::rect> B;
typedef rect R;

namespace augs {
	template <bool C>
	template <class>
	basic_handle<C, B, R>::operator basic_handle<true, B, R>() const {
		return basic_handle<true, B, R>(owner, raw_id);
	}

	template <bool C>
	template <class>
	void basic_handle<C, B, R>::scroll_content_with_wheel(event_info e) {
		auto& pool = get_pool();
		auto& self = get();

		auto& sys = e.owner;
		auto& wnd = sys.state;
		if (e == gui_event::wheel) {
			if (wnd.keys[augs::window::event::keys::SHIFT]) {
				int temp(int(self.scroll.x));
				if (self.scrollable) {
					self.scroll.x -= wnd.mouse.scroll;
					clamp_scroll_to_right_down_corner();
				}
				if ((!self.scrollable || temp == self.scroll.x) && get_parent().alive()) {
					get_parent().consume_gui_event(e = gui_event::wheel);
				}
			}
			else {
				int temp(int(self.scroll.y));
				if (self.scrollable) {
					self.scroll.y -= wnd.mouse.scroll;
					clamp_scroll_to_right_down_corner();
				}
				if ((!self.scrollable || temp == self.scroll.y) && get_parent().alive()) {
					get_parent().consume_gui_event(e = gui_event::wheel);
				}
			}
		}
	}

	template <bool C>
	template <class>
	void basic_handle<C, B, R>::try_to_enable_middlescrolling(event_info e) {
		auto& pool = get_pool();
		auto& self = get();

		auto& gr = e.owner;
		auto& wnd = gr.state;
		if (e == gui_event::mdown || e == gui_event::mdoubleclick) {
			if (self.scrollable && !self.content_size.inside(rects::wh<float>(self.rc))) {
				gr.middlescroll.subject = *this;
				gr.middlescroll.pos = wnd.mouse.pos;
				gr.set_focus(*this);
			}
			else if (get_parent().alive()) {
				get_parent().consume_gui_event(e);
			}
		}
	}

	template <bool C>
	template <class>
	void basic_handle<C, B, R>::try_to_make_this_rect_focused(event_info e) {
		auto& pool = get_pool();

		if (!focusable) return;
		auto& sys = e.owner;
		if (e == gui_event::ldown ||
			e == gui_event::ldoubleclick ||
			e == gui_event::ltripleclick ||
			e == gui_event::rdoubleclick ||
			e == gui_event::rdown
			) {
			if (pool[sys.get_rect_in_focus()].alive()) {
				if (get().preserve_focus || !pool[sys.get_rect_in_focus()].get().preserve_focus)
					sys.set_focus(*this);
			}
			else sys.set_focus(*this);
		}
	}

	template <bool C>
	template <class>
	void basic_handle<C, B, R>::scroll_to_view() const {
		if (get_parent().alive()) {
			auto& p = get_parent().get();

			rects::ltrb<float> global = get().get_rect_absolute();
			rects::ltrb<float> parent_global = p.get_rect_absolute();
			vec2i off1 = vec2i(std::max(0.f, global.r + 2 - parent_global.r), std::max(0.f, global.b + 2 - parent_global.b));
			vec2i off2 = vec2i(std::max(0.f, parent_global.l - global.l + 2 + off1.x), std::max(0.f, parent_global.t - global.t + 2 + off1.y));
			p.scroll += off1;
			p.scroll -= off2;
			get_parent().scroll_to_view();
		}
	}

	template <bool C>
	basic_handle<C, B, R> basic_handle<C, B, R>::get_parent() const {
		return get_pool()[get().parent];
	}


	template <bool C>
	bool basic_handle<C, B, R>::is_being_dragged(rect_world& g) const {
		return g.rect_held_by_lmb == *this && g.held_rect_is_dragged;
	}
	
	template <bool C>
	std::vector<basic_handle<C, B, R>> basic_handle<C, B, R>::get_children() const {
		return get_pool()[get().children];
	}
}

// explicit instantiation
template class augs::basic_handle<true, B, R>;
template class augs::basic_handle<false, B, R>;