#pragma once
#include <functional>
#include "augs/misc/pool_handle.h"
#include "augs/misc/pool.h"
#include "rect_id.h"
#include "material.h"
#include "stylesheet.h"
#include "draw_and_event_infos.h"

namespace augs {
	namespace gui {
		struct rect;
		class rect_world;

		template<bool is_const, class derived>
		class default_rect_callbacks {
		public:
			template<bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
			void logic() const {
				
			}

			template<bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
			void event(event_info e) const {
				const auto& self = *static_cast<const derived*>(this);
				auto r = self.get_rect();

				r.try_to_enable_middlescrolling(e);
				r.try_to_make_this_rect_focused(e);
				r.scroll_content_with_wheel(e);
			}

			void draw(draw_info) const {

			}

			rects::wh<float> content_size() const {
				const auto& self = *static_cast<const derived*>(this);
				auto r = self.get_rect();

				/* init on zero */
				rects::ltrb<float> content = rects::ltrb<float>(0, 0, 0, 0);

				/* enlarge the content size by every child */
				auto children_all = r.get_children();
				for (size_t i = 0; i < children_all.size(); ++i)
					if (children_all[i].get().enable_drawing)
						content.contain_positive(children_all[i].get().rc);

				return content;
			}
		};
	}

	template <bool is_const>
	class basic_handle<is_const, basic_pool<gui::rect>, gui::rect> :
		public basic_handle_base<is_const, basic_pool<gui::rect>, gui::rect> {
	public:
		typedef basic_handle<is_const, basic_pool<gui::rect>, gui::rect> handle_type;

		using basic_handle_base<is_const, basic_pool<gui::rect>, gui::rect>::basic_handle_base;
		using basic_handle_base<is_const, basic_pool<gui::rect>, gui::rect>::operator pool_id<gui::rect>;

		template <bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
		operator basic_handle<true, basic_pool<gui::rect>, gui::rect>() const {
			return basic_handle<true, augs::basic_pool<augs::gui::rect>, augs::gui::rect>(this->owner, this->raw_id);
		}

		template <class D, bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
		void consume_raw_input_and_generate_gui_events(gui::raw_event_info&, D dispatcher) const {
			using namespace augs::window::event;
			auto& gr = this->inf.owner;
			auto& m = this->gr.state.mouse;
			unsigned msg = this->inf.msg;
      gui::event_info e(gr, gui_event::unknown);

			auto& pool = this->get_pool();
			auto& self = this->get();

			if (self.enable_drawing) {
				if (self.enable_drawing_of_children) {
					auto children_all = get_children();
					for (int i = children_all.size() - 1; i >= 0; --i) {
						if (!children_all[i].get().enable_drawing) continue;
						children_all[i].get().parent = *this;
						children_all[i].consume_raw_input_and_generate_gui_events(this->inf, dispatcher);
					}
				}

				if (!self.disable_hovering) {
					if (gr.rect_hovered == nullptr) {
						bool hover = self.rc_clipped.good() && self.rc_clipped.hover(m.pos);

						if (hover) {
							consume_gui_event(e = gui_event::hover, dispatcher);
							gr.rect_hovered = this;
							gr.was_hovered_rect_visited = true;
						}
					}
					else if (gr.rect_hovered == this) {
						bool still_hover = self.rc_clipped.good() && self.rc_clipped.hover(m.pos);

						if (still_hover) {
							if (msg == lup) {
								consume_gui_event(e = gui_event::lup, dispatcher);
							}
							if (msg == ldown) {
								gr.rect_held_by_lmb = this;
								gr.ldrag_relative_anchor = m.pos - this->rc.get_position();
								gr.last_ldown_position = m.pos;
								this->rc_pos_before_dragging = vec2i(this->rc.l, this->rc.t);
								consume_gui_event(e = gui_event::ldown, dispatcher);
							}
							if (msg == mdown) {
								consume_gui_event(e = gui_event::mdown, dispatcher);
							}
							if (msg == mdoubleclick) {
								consume_gui_event(e = gui_event::mdoubleclick, dispatcher);
							}
							if (msg == ldoubleclick) {
								gr.rect_held_by_lmb = this;
								gr.ldrag_relative_anchor = m.pos - this->rc.get_position();
								gr.last_ldown_position = m.pos;
								consume_gui_event(e = gui_event::ldoubleclick, dispatcher);
							}
							if (msg == ltripleclick) {
								gr.rect_held_by_lmb = this;
								gr.ldrag_relative_anchor = m.pos - this->rc.get_position();
								gr.last_ldown_position = m.pos;
								consume_gui_event(e = gui_event::ltripleclick, dispatcher);
							}
							if (msg == rdown) {
								gr.rect_held_by_rmb = this;
								consume_gui_event(e = gui_event::rdown, dispatcher);
							}
							if (msg == rdoubleclick) {
								gr.rect_held_by_rmb = this;
								consume_gui_event(e = gui_event::rdoubleclick, dispatcher);
							}

							if (msg == wheel) {
								consume_gui_event(e = gui_event::wheel, dispatcher);
							}

							if (gr.rect_held_by_lmb == this && msg == mousemotion && m.state[0] && self.rc_clipped.hover(m.ldrag)) {
								consume_gui_event(e = gui_event::lpressed, dispatcher);
							}
							if (gr.rect_held_by_rmb == this && msg == mousemotion && m.state[1] && self.rc_clipped.hover(m.rdrag)) {
								consume_gui_event(e = gui_event::rpressed, dispatcher);
							}
						}
						else {
							// ensure(msg == mousemotion);
							consume_gui_event(e = gui_event::hout, dispatcher);
							unhover(this->inf, dispatcher);
						}

						gr.was_hovered_rect_visited = true;
					}
				}

				if (gr.rect_held_by_lmb == this && msg == mousemotion && m.pos != gr.last_ldown_position) {
					gr.held_rect_is_dragged = true;
					gr.current_drag_amount = m.pos - gr.last_ldown_position;
					consume_gui_event(e = gui_event::ldrag);
				}
			}
		}

		template <class D, bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
		void consume_gui_event(gui::event_info e, D dispatcher) const {			
			dispatcher.dispatch(*this, [&e](auto c) {
				c.event(e);
			});
		}

		template <class D, bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
		void calculate_clipped_rectangle_layout(D dispatcher) const {
			/* init; later to be processed absolute and clipped with local rc */
			auto& self = this->get();

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

			self.content_size = dispatcher.dispatch(*this, [self](auto handle) {
				self.content_size = handle.content_size();
			});

			/* align scroll only to be positive and not to exceed content size */
			if (self.snap_scroll_to_content_size)
				this->clamp_scroll_to_right_down_corner();

			/* do the same for every child */
			auto children_all = get_children();
			for (size_t i = 0; i < children_all.size(); ++i) {
				children_all[i].get().parent = this;
				//if (children_all[i]->enable_drawing)
				children_all[i].calculate_clipped_rectangle_layout(this->behaviour);
			}
		}

		template <class D, bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
		void perform_logic_step(D dispatcher) const {
			
			dispatcher.dispatch(*this, [](auto c) {
				c.logic();
			});

			auto children_all = get_children();

			for (size_t i = 0; i < children_all.size(); ++i) {
				children_all[i].get().parent = *this;
				children_all[i].perform_logic_step(dispatcher);
			}
		}

		/* consume_gui_event default subroutines */
		template <bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
		void scroll_content_with_wheel(gui::event_info) {
			auto& self = this->get();

			auto& sys = this->e.owner;
			auto& wnd = this->sys.state;
			if (this->e == gui_event::wheel) {
				if (wnd.keys[augs::window::event::keys::SHIFT]) {
					int temp(int(self.scroll.x));
					if (self.scrollable) {
						self.scroll.x -= wnd.mouse.scroll;
						this->clamp_scroll_to_right_down_corner();
					}
					if ((!self.scrollable || temp == self.scroll.x) && get_parent().alive()) {
						get_parent().consume_gui_event(this->e = gui_event::wheel);
					}
				}
				else {
					int temp(int(self.scroll.y));
					if (self.scrollable) {
						self.scroll.y -= wnd.mouse.scroll;
						this->clamp_scroll_to_right_down_corner();
					}
					if ((!self.scrollable || temp == self.scroll.y) && get_parent().alive()) {
						get_parent().consume_gui_event(this->e = gui_event::wheel);
					}
				}
			}
		}

		template <bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
		void try_to_enable_middlescrolling(gui::event_info) {
			auto& self = this->get();

			auto& gr = this->e.owner;
			auto& wnd = this->gr.state;
			if (this->e == gui_event::mdown || this->e == gui_event::mdoubleclick) {
				if (self.scrollable && !self.content_size.inside(rects::wh<float>(self.rc))) {
					gr.middlescroll.subject = *this;
					gr.middlescroll.pos = wnd.mouse.pos;
					gr.set_focus(*this);
				}
				else if (get_parent().alive()) {
					get_parent().consume_gui_event(this->e);
				}
			}
		}

		template <bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
		void try_to_make_this_rect_focused(gui::event_info) {
			auto& pool = this->get_pool();

			if (!this->focusable) return;
			auto& sys = this->e.owner;
			if (this->e == gui_event::ldown ||
				this->e == gui_event::ldoubleclick ||
				this->e == gui_event::ltripleclick ||
				this->e == gui_event::rdoubleclick ||
				this->e == gui_event::rdown
				) {
				if (pool[sys.get_rect_in_focus()].alive()) {
					if (this->get().preserve_focus || !pool[sys.get_rect_in_focus()].get().preserve_focus)
						sys.set_focus(*this);
				}
				else sys.set_focus(*this);
			}
		}

		/* try to scroll to view whole content */
		template <bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
		void scroll_to_view() const {
			if (get_parent().alive()) {
				auto& p = get_parent().get();

				rects::ltrb<float> global = this->get().get_rect_absolute();
				rects::ltrb<float> parent_global = p.get_rect_absolute();
				vec2i off1 = vec2i(std::max(0.f, global.r + 2 - parent_global.r), std::max(0.f, global.b + 2 - parent_global.b));
				vec2i off2 = vec2i(std::max(0.f, parent_global.l - global.l + 2 + off1.x), std::max(0.f, parent_global.t - global.t + 2 + off1.y));
				p.scroll += off1;
				p.scroll -= off2;
				get_parent().scroll_to_view();
			}
		}

		/* draw_triangles default subroutines */
		void draw_stretched_texture(gui::draw_info in, const gui::material& = gui::material()) const;
		
		void draw_centered_texture(gui::draw_info in, const gui::material& = gui::material(), vec2i offset = vec2i()) const;
		
		void draw_rectangle_stylesheeted(gui::draw_info in, const gui::stylesheet&) const;

		std::vector<handle_type> get_children() const;
		handle_type get_parent() const;
		
		bool is_being_dragged(gui::rect_world&) const;

		template <class D, bool _is_const = is_const, class = std::enable_if_t<!_is_const>>
		void unhover(gui::raw_event_info& inf, D dispatcher) const {
      gui::event_info e(inf.owner, gui_event::unknown);

			consume_gui_event(e = gui_event::hoverlost, dispatcher);

			if (this->inf.owner.rect_held_by_lmb == this)
				consume_gui_event(e = gui_event::loutdrag, dispatcher);

			this->inf.owner.rect_hovered = this->rect_id();
		}
		
		template <class D>
		void draw_children(gui::draw_info in, D dispatcher) const {
			auto& self = this->get();

			if (!self.enable_drawing_of_children)
				return;

			auto children_all = get_children();
			for (size_t i = 0; i < children_all.size(); ++i) {
				if (children_all[i].get().enable_drawing) {
					
					dispatcher.dispatch(children_all[i], [in](auto c) {
						c.draw(in);
					});

					children_all[i].draw_children(in, this->behaviour);
				}
			}
		}
	};
}
