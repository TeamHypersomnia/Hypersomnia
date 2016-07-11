#pragma once
#include "rect.h"
#include "stylesheet.h"
#include "window_framework/window.h"

#include <algorithm>
#include <functional>
#include "ensure.h"
#undef max
#undef min
namespace augs {
	namespace gui {
		rect::draw_info::draw_info(const rect_world& owner, vertex_triangle_buffer& v) : owner(owner), v(v) {}
		rect::poll_info::poll_info(rect_world& owner, unsigned msg) : owner(owner), msg(msg), mouse_fetched(false), scroll_fetched(false) {}
		rect::event_info::event_info(rect_world& owner, rect::gui_event msg) : owner(owner), msg(msg) {}

		rect::event_info::operator rect::gui_event() {
			return msg;
		}

		rect::event_info& rect::event_info::operator=(gui_event m) {
			msg = m;
			return *this;
		}


		rect::rect(rects::xywh<float> rc) : rc(rc) {}
		rect::rect(assets::texture_id id) {
			rc.set_size((*id).get_size());
		 }

		rects::wh<float> rect::get_content_size() const {
			/* init on zero */
			rects::ltrb<float> content = rects::ltrb<float>(0, 0, 0, 0);

			/* enlarge the content size by every child */
			auto children_all = children;
			for (size_t i = 0; i < children_all.size(); ++i)
				if (children_all[i]->enable_drawing)
					content.contain_positive(children_all[i]->rc);

			return content;
		}

		void rect::calculate_clipped_rectangle_layout() {
			/* init; later to be processed absolute and clipped with local rc */
			rc_clipped = rc;
			absolute_xy = vec2i(rc.l, rc.t);

			/* if we have parent */
			if (parent) {
				/* we have to save our global coordinates in absolute_xy */
				absolute_xy = parent->absolute_xy + vec2i(rc.l, rc.t) - vec2i(int(parent->scroll.x), int(parent->scroll.y));
				rc_clipped = rects::xywh<float>(absolute_xy.x, absolute_xy.y, rc.w(), rc.h());

				/* and we have to clip by first clipping parent's rc_clipped */
				//auto* clipping = get_clipping_parent(); 
				//if(clipping) 
				rc_clipped.clip_by(parent->clipping_rect);

				clipping_rect = parent->clipping_rect;
			}

			if (clip)
				clipping_rect.clip_by(rc_clipped);

			/* update content size */
			content_size = this->get_content_size();

			/* align scroll only to be positive and not to exceed content size */
			if (snap_scroll_to_content_size)
				clamp_scroll_to_right_down_corner();

			/* do the same for every child */
			auto children_all = children;
			for (size_t i = 0; i < children_all.size(); ++i) {
				children_all[i]->parent = this;
				//if (children_all[i]->enable_drawing)
					children_all[i]->calculate_clipped_rectangle_layout();
			}
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

		void rect::draw_children(draw_info in) const {
			if (!enable_drawing_of_children) 
				return;

			auto children_all = children;
			for (size_t i = 0; i < children_all.size(); ++i) {
				if (children_all[i]->enable_drawing)
					children_all[i]->draw_triangles(in);
			}
		}

		void rect::perform_logic_step(rect_world& owner) {
			auto children_all = children;
			for (size_t i = 0; i < children_all.size(); ++i) {
				children_all[i]->parent = this;
				children_all[i]->perform_logic_step(owner);
			}
		}

		void rect::draw_triangles(draw_info in) {
			draw_children(in);
		}

		/* handle focus and passing scroll to parents */

		void rect::scroll_content_with_wheel(event_info e) {
			auto& sys = e.owner;
			auto& wnd = sys.state;
			if (e == gui_event::wheel) {
				if (wnd.keys[augs::window::event::keys::SHIFT]) {
					int temp(int(scroll.x));
					if (scrollable) {
						scroll.x -= wnd.mouse.scroll;
						clamp_scroll_to_right_down_corner();
					}
					if ((!scrollable || temp == scroll.x) && parent) {
						parent->consume_gui_event(e = gui_event::wheel);
					}
				}
				else {
					int temp(int(scroll.y));
					if (scrollable) {
						scroll.y -= wnd.mouse.scroll;
						clamp_scroll_to_right_down_corner();
					}
					if ((!scrollable || temp == scroll.y) && parent) {
						parent->consume_gui_event(e = gui_event::wheel);
					}
				}
			}
		}

		void rect::try_to_enable_middlescrolling(event_info e) {
			auto& gr = e.owner;
			auto& wnd = gr.state;
			if (e == gui_event::mdown || e == gui_event::mdoubleclick) {
				if (scrollable && !content_size.inside(rects::wh<float>(rc))) {
					gr.middlescroll.subject = this;
					gr.middlescroll.pos = wnd.mouse.pos;
					gr.set_focus(this);
				}
				else if (parent) {
					parent->consume_gui_event(e);
				}
			}
		}

		void rect::try_to_make_this_rect_focused(event_info e) {
			if (!focusable) return;
			auto& sys = e.owner;
			if (e == gui_event::ldown ||
				e == gui_event::ldoubleclick ||
				e == gui_event::ltripleclick ||
				e == gui_event::rdoubleclick ||
				e == gui_event::rdown
				) {
				if (sys.get_rect_in_focus() != nullptr) {
					if (preserve_focus || !sys.get_rect_in_focus()->preserve_focus)
						sys.set_focus(this);
				}
				else sys.set_focus(this);
			}
		}

		bool rect::focus_next_rect_by_tab(event_info e) {
			using namespace augs::window::event::keys;
			if (e == gui_event::keydown && e.owner.state.key == TAB) {
				rect_id f = seek_focusable(this, e.owner.state.keys[LSHIFT]);
				if (f) e.owner.set_focus(f);
				return true;
			}
			/* in case it's character event */
			if (e == gui_event::character && e.owner.state.key == TAB) return true;
			return false;
		}

		bool rect::focus_next_rect_by_arrows(event_info e) {
			using namespace augs::window::event::keys;
			if (e == gui_event::keydown) {
				rect_id f = nullptr;
				switch (e.owner.state.key) {
				case DOWN: f = seek_focusable(this, false); break;
				case UP: f = seek_focusable(this, true); break;
				case LEFT: f = seek_focusable(this, true); break;
				case RIGHT: f = seek_focusable(this, false); break;
				default: break;
				}

				if (f)
					e.owner.set_focus(f);

				return true;
			}

			return false;
		}

		bool rect::focus_next_rect_by_enter(event_info e) {
			using namespace augs::window::event::keys;
			if (e == gui_event::keydown && e.owner.state.key == ENTER) {
				rect_id f = seek_focusable(this, e.owner.state.keys[LSHIFT]);
				if (f) e.owner.set_focus(f);
				return true;
			}
			/* in case it's character event */
			if (e == gui_event::character && e.owner.state.key == ENTER) return true;
			return false;
		}

		rect_id rect::seek_focusable(rect_id f, bool prev) {
			rect_id rect::*fn = prev ? &rect::prev_focusable : &rect::next_focusable;
			rect_id it = f;
			if (f->preserve_focus) return nullptr;
			while (true) {
				it = it->*fn;
				if (!it || it == f) return nullptr;
				if (it->focusable) return it;
			}
		}
		
		void rect::consume_gui_event(event_info e) {
			try_to_enable_middlescrolling(e);
			try_to_make_this_rect_focused(e);
			scroll_content_with_wheel(e);
			focus_next_rect_by_tab(e);
			focus_next_rect_by_arrows(e);
		}

		void rect::consume_raw_input_and_generate_gui_events(poll_info& inf) {
			using namespace augs::window::event;
			auto& gr = inf.owner;
			auto& m = gr.state.mouse;
			unsigned msg = inf.msg;
			event_info e(gr, gui_event::unknown);

			if (enable_drawing) {
				if (enable_drawing_of_children) {
					auto children_all = children;
					for (int i = children_all.size() - 1; i >= 0; --i) {
						if (!children_all[i]->enable_drawing) continue;
						children_all[i]->parent = this;
						children_all[i]->consume_raw_input_and_generate_gui_events(inf);
					}
				}

				if (!disable_hovering) {
					if (gr.rect_hovered == nullptr) {
						bool hover = rc_clipped.good() && rc_clipped.hover(m.pos);

						if (hover) {
							consume_gui_event(e = gui_event::hover);
							gr.rect_hovered = this;
							gr.was_hovered_rect_visited = true;
						}
					}
					else if (gr.rect_hovered == this) {
						bool still_hover = rc_clipped.good() && rc_clipped.hover(m.pos);

						if (still_hover) {
							if (msg == lup) {
								consume_gui_event(e = gui_event::lup);
							}
							if (msg == ldown) {
								gr.rect_held_by_lmb = this;
								gr.ldrag_relative_anchor = m.pos - rc.get_position();
								gr.last_ldown_position = m.pos;
								rc_pos_before_dragging = vec2i(rc.l, rc.t);
								consume_gui_event(e = gui_event::ldown);
							}
							if (msg == mdown) {
								consume_gui_event(e = gui_event::mdown);
							}
							if (msg == mdoubleclick) {
								consume_gui_event(e = gui_event::mdoubleclick);
							}
							if (msg == ldoubleclick) {
								gr.rect_held_by_lmb = this;
								gr.ldrag_relative_anchor = m.pos - rc.get_position();
								gr.last_ldown_position = m.pos;
								consume_gui_event(e = gui_event::ldoubleclick);
							}
							if (msg == ltripleclick) {
								gr.rect_held_by_lmb = this;
								gr.ldrag_relative_anchor = m.pos - rc.get_position();
								gr.last_ldown_position = m.pos;
								consume_gui_event(e = gui_event::ltripleclick);
							}
							if (msg == rdown) {
								gr.rect_held_by_rmb = this;
								consume_gui_event(e = gui_event::rdown);
							}
							if (msg == rdoubleclick) {
								gr.rect_held_by_rmb = this;
								consume_gui_event(e = gui_event::rdoubleclick);
							}

							if (msg == wheel) {
								consume_gui_event(e = gui_event::wheel);
							}

							if (gr.rect_held_by_lmb == this && msg == mousemotion && m.state[0] && rc_clipped.hover(m.ldrag)) {
								consume_gui_event(e = gui_event::lpressed);
							}
							if (gr.rect_held_by_rmb == this && msg == mousemotion && m.state[1] && rc_clipped.hover(m.rdrag)) {
								consume_gui_event(e = gui_event::rpressed);
							}
						}
						else {
							// ensure(msg == mousemotion);
							consume_gui_event(e = gui_event::hout);
							unhover(inf);
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

		void rect::unhover(poll_info& inf) {
			event_info e(inf.owner, gui_event::unknown);

			consume_gui_event(e = gui_event::hoverlost);

			if (inf.owner.rect_held_by_lmb == this)
				consume_gui_event(e = gui_event::loutdrag);

			inf.owner.rect_hovered = nullptr;
		}

		bool rect::is_scroll_clamped_to_right_down_corner() {
			return rects::wh<float>(rc).can_contain(content_size, scroll);
		}

		void rect::clamp_scroll_to_right_down_corner() {
			rects::wh<float>(rc).clamp_offset_to_right_down_corner_of(content_size, scroll);
		}

		void rect::scroll_to_view() {
			if (parent) {
				rects::ltrb<float> global = get_rect_absolute();
				rects::ltrb<float> parent_global = parent->get_rect_absolute();
				vec2i off1 = vec2i(std::max(0.f, global.r + 2 - parent_global.r), std::max(0.f, global.b + 2 - parent_global.b));
				vec2i off2 = vec2i(std::max(0.f, parent_global.l - global.l + 2 + off1.x), std::max(0.f, parent_global.t - global.t + 2 + off1.y));
				parent->scroll += off1;
				parent->scroll -= off2;
				parent->scroll_to_view();
			}
		}

		void rect::gen_focus_links() {
			auto children_all = children;
			if (children_all.empty()) {
				//if(next) next_focusable = next;
				return;
			}

			auto order = children_all;

			//for(unsigned i = 0; i < order.size(); ++i) {
			//	if(!order[i]->focusable) {
			//		order.erase(order.begin()+i);
			//		--i;
			//	}
			//}

			/* operations on order, sort by vertical distance */
			std::sort(order.begin(), order.end(), [](rect_id a, rect_id b) {
				auto& r1 = a->rc;
				auto& r2 = b->rc;
				return (r1.t == r2.t) ? (r1.l < r2.l) : (r1.t < r2.t);
			});

			//if(next == nullptr) next = this;
				//next = focusable ? this : order[0];


			next_focusable = order[0];
			prev_focusable = *order.rbegin();

			for (unsigned i = 0; i < order.size(); ++i) {
				order[i]->next_focusable = (i == order.size() - 1) ? this : order[i + 1];
				order[i]->prev_focusable = (i == 0) ? this : order[i - 1];
			}
		}

		void rect::gen_focus_links_depth(rect_id next) {
			auto children_all = children;

			if (next == nullptr)
				next = this;

			if (children_all.empty()) {
				next_focusable = next;
			}

			auto order = children_all;

			/* operations on order, sort by vertical distance */
			sort(order.begin(), order.end(), [](rect_id a, rect_id b) {
				auto& r1 = a->rc;
				auto& r2 = b->rc;
				return (r1.t == r2.t) ? (r1.l < r2.l) : (r1.t < r2.t);
			});

			if (!children_all.empty())
				next_focusable = order[0];

			for (unsigned i = 0; i < order.size(); ++i) {
				order[i]->gen_focus_links_depth((i == order.size() - 1) ? next : order[i + 1]);
			}
		}
		
		bool rect::is_being_dragged(rect_world& g) {
			return g.rect_held_by_lmb == this && g.held_rect_is_dragged;
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

		rect_id rect::get_parent() const {
			return parent;
		}

		rects::ltrb<float> rect::get_rect_absolute() const {
			return rects::xywh<float>(absolute_xy.x, absolute_xy.y, rc.w(), rc.h());
		}

		const vec2i& rect::get_absolute_xy() const {
			return absolute_xy;
		}
	}
}
