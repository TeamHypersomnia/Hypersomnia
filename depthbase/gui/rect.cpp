#pragma once
#include "rect.h"
#include "stylesheet.h"
#include <algorithm>
#include <functional>
#undef max
#undef min
namespace augs {
	namespace graphics {
		namespace gui {
			rect::draw_info::draw_info(group& owner, std::vector<quad>& v) : owner(owner), v(v) {}
			rect::poll_info::poll_info(group& owner, unsigned msg) : owner(owner), msg(msg), mouse_fetched(false), scroll_fetched(false) {}
			rect::event_info::event_info(group& owner, rect::event msg) : owner(owner), msg(msg) {}
			
			rect::event_info::operator rect::event() {
				return msg;
			}
					
			rect::event_info& rect::event_info::operator=(event m) {
				msg = m;
				return *this;
			}


			rect::rect(const rects::xywh<float>& rc) 
				: rc(rc), parent(nullptr), snap_scroll_to_content(true),
				draw(true), was_hovered(false), fetch_wheel(false), clip(true), scrollable(true), content_size(rects::wh<float>()), rc_clipped(rc), preserve_focus(false), focusable(true),
				focus_next(nullptr), focus_prev(nullptr), clipping_rect(0, 0, std::numeric_limits<int>::max()/2, std::numeric_limits<int>::max()/2) 
			{
				quad_indices.background = -1;
			}
			
			rects::wh<float> rect::get_content_size() {
				/* init on zero */
				rects::ltrb<float> content = rects::ltrb<float>(0, 0, 0, 0);
				
				/* enlarge the content size by every child */
				auto children_all = children;
				get_member_children(children_all);
				for(size_t i = 0; i < children_all.size(); ++i)
					if(children_all[i]->draw)
				 		content.contain_positive(children_all[i]->rc);
				
				return content;
			}

			void rect::update_rectangles() {
				/* init; later to be processed absolute and clipped with local rc */
				rc_clipped = rc;
				absolute_xy = vec2<int>(rc.l, rc.t);

				/* if we have parent */
				if(parent) {
					/* we have to save our global coordinates in absolute_xy */
					absolute_xy = parent->absolute_xy + vec2<int>(rc.l, rc.t) - vec2<int>(int(parent->scroll.x), int(parent->scroll.y));
 					rc_clipped  = rects::xywh<float>(absolute_xy.x, absolute_xy.y, rc.w(), rc.h());
					
					/* and we have to clip by first clipping parent's rc_clipped */
					//auto* clipping = get_clipping_parent(); 
					//if(clipping) 
					rc_clipped.clip(parent->clipping_rect);
					
					clipping_rect = parent->clipping_rect;
				}

				if(clip)
					clipping_rect.clip(rc_clipped);
					
				/* update content size */
				content_size = this->get_content_size();
				
				/* align scroll only to be positive and not to exceed content size */
				if(snap_scroll_to_content)
					align_scroll();

				/* do the same for every child */
				auto children_all = children;
				get_member_children(children_all);
				for(size_t i = 0; i < children_all.size(); ++i) {
					children_all[i]->parent = this;
					if(children_all[i]->draw)
					   children_all[i]->update_rectangles();
				}
			}
			
			void rect::draw_rect(draw_info in, const material& mat) {
				quad_indices.background = in.v.size();
				if(!add_quad(mat, get_rect_absolute(), parent, in.v).good()) quad_indices.background = -1;
				// rc_clipped = add_quad(mat, rc_clipped, parent, in.v);
			}
			
			void rect::draw_rect(draw_info in, const stylesheet& styles) {
				auto st = styles.get_style();

				if(st.color.active || st.background_image.active)
					draw_rect(in, material(st));
				
				if(st.border.active) st.border.value.draw(in.v, *this);
			}

			void rect::draw_children(draw_info in) {
				auto children_all = children;
				get_member_children(children_all);
				for(size_t i = 0; i < children_all.size(); ++i) { 
					children_all[i]->parent = this;
					if(children_all[i]->draw)
					   children_all[i]->draw_proc(in);
				}
			}
			 
			void rect::get_member_children(std::vector<rect*>& children) {

			}
			
			void rect::update_proc(group& owner) {
				auto children_all = children;
				get_member_children(children_all);
				for(size_t i = 0; i < children_all.size(); ++i) { 
					children_all[i]->parent = this;
					children_all[i]->update_proc(owner);
				}
			}
			
			void rect::draw_proc(draw_info in) {
				draw_children(in);
			}

			/* handle focus and passing scroll to parents */
			
			void rect::handle_scroll(event_info e) {
				auto& sys =	e.owner.owner;
				auto& wnd = sys.events;
				if(e == event::wheel) {
					if(wnd.keys[augs::window::event::keys::SHIFT]) {
						int temp(int(scroll.x));
						if(scrollable) {
							scroll.x -= wnd.mouse.scroll;
							align_scroll();
						}
						if((!scrollable || temp == scroll.x) && parent) {
							parent->event_proc(e = event::wheel);
						}
					}
					else {
						int temp(int(scroll.y));
						if(scrollable) {
							scroll.y -= wnd.mouse.scroll;
							align_scroll();
						}
						if((!scrollable || temp == scroll.y) && parent) {
							parent->event_proc(e = event::wheel);
						}
					}
				}
			}

			void rect::handle_middleclick(event_info e) {
				auto& gr = e.owner;
				auto& wnd = gr.owner.events;
				if(e == event::mdown || e == event::mdoubleclick) {
					if(scrollable && !content_size.inside(rects::wh<float>(rc))) {
						gr.middlescroll.subject = this;
						gr.middlescroll.pos = wnd.mouse.pos;
						gr.set_focus(this);
					} else if(parent) {
						parent->event_proc(e);
					}
				}
			}

			void rect::handle_focus(event_info e) {
				if(!focusable) return;
				auto& sys =	 e.owner;
				if(e == event::ldown ||
					e == event::ldoubleclick ||
					e == event::ltripleclick ||
					e == event::rdoubleclick ||
					e == event::rdown
					) {
						if(sys.get_focus()) {
							if(preserve_focus || !sys.get_focus()->preserve_focus)
								sys.set_focus(this);
						}
						else sys.set_focus(this);
				}
			}
			
			bool rect::handle_tab(event_info e) {
				using namespace augs::window::event::keys;
				if(e == event::keydown && e.owner.owner.events.key == TAB) {
					rect* f = seek_focusable(this, e.owner.owner.events.keys[LSHIFT]);
					if(f) e.owner.set_focus(f);
					return true;
				}
				/* in case it's character event */
				if(e == event::character && e.owner.owner.events.key == TAB) return true;
				return false;
			}

			bool rect::handle_arrows(event_info e) {
				using namespace augs::window::event::keys;
				if(e == event::keydown) {
					rect* f = nullptr;
					switch(e.owner.owner.events.key) {
					case DOWN: f = seek_focusable(this, false); break;
					case UP: f = seek_focusable(this, true); break;
					case LEFT: f = seek_focusable(this, true); break;
					case RIGHT: f = seek_focusable(this, false); break;
					default: break;
					}

					if(f) 
						e.owner.set_focus(f);

					return true;
				}

				return false;
			}

			bool rect::handle_enter(event_info e) {
				using namespace augs::window::event::keys;
				if(e == event::keydown && e.owner.owner.events.key == ENTER) {
					rect* f = seek_focusable(this, e.owner.owner.events.keys[LSHIFT]);
					if(f) e.owner.set_focus(f);
					return true;
				}
				/* in case it's character event */
				if(e == event::character && e.owner.owner.events.key == ENTER) return true;
				return false;
			}
			
			rect* rect::seek_focusable(rect* f, bool prev) {
				rect* rect::*fn = prev ? &rect::focus_prev : &rect::focus_next; 
				rect* it = f;
				if(f->preserve_focus) return nullptr;
				while(true) {
					it = it->*fn;
					if(!it || it == f) return nullptr; 
					if(it->focusable) return it;
				}
			}

			void rect::event_proc(event_info e) {
				handle_middleclick(e);
				handle_focus(e);
				handle_scroll(e);
				handle_tab(e);
				handle_arrows(e);
			}

			void rect::poll_message(poll_info& inf) {
				using namespace augs::window::event;
				using namespace mouse;
				auto& gr = inf.owner;
				auto& m = gr.owner.events.mouse;
				unsigned msg = inf.msg;
				event_info e(gr, event::unknown);

				if(draw) {
					auto children_all = children;
					get_member_children(children_all);
					for(int i = children_all.size()-1; i >= 0; --i) {
						if(!children_all[i]->draw) continue;
						children_all[i]->parent = this;
						children_all[i]->poll_message(inf);
					}

					//if(msg == key::down) event_proc(e = event::keydown); 
					//else if(msg == key::up) event_proc(e = event::keyup); 
					//else if(msg == key::character) event_proc(e = event::character); 
					//else if(msg == key::unichar) event_proc(e = event::unichar); 
					//else {
						bool hover = rc_clipped.hover(m.pos);

						if(hover && !inf.mouse_fetched) {
							if(!was_hovered) 
								event_proc(e = event::hover);
							
							inf.mouse_fetched = was_hovered = true;
							if(msg == lup) {
								event_proc(e = event::lup);	
							}
							if(msg == ldown) {
								gr.lholded = this;
								event_proc(e = event::ldown);	
							}
							if(msg == mdown) {
								event_proc(e = event::mdown);	
							}
							if(msg == mdoubleclick) {
								event_proc(e = event::mdoubleclick);	
							}
							if(msg == ldoubleclick) {
								gr.lholded = this;
								event_proc(e = event::ldoubleclick);	
							}
							if(msg == ltripleclick) {
								gr.lholded = this;
								event_proc(e = event::ltripleclick);	
							}
							if(msg == rdown) {
								gr.rholded = this;
								event_proc(e = event::rdown);
							}
							if(msg == rdoubleclick) {
								gr.rholded = this;
								event_proc(e = event::rdoubleclick);
							}

							if(msg == wheel) {
								event_proc(e = event::wheel);
							}

							if(gr.lholded == this && msg == motion && m.state[0] && rc_clipped.hover(m.ldrag)) {
								gr.lholded = this;
								event_proc(e = event::lpressed);
							}
							if(gr.rholded == this && msg == motion && m.state[1] && rc_clipped.hover(m.rdrag)) {
								gr.rholded = this;
								event_proc(e = event::rpressed);
							}
						}
						else if(hover) {

						}
						else if(!hover) {
							if(msg == motion) {
								if(was_hovered) {
									event_proc(e = event::hout);
									was_hovered = false;
								}
								if(gr.lholded == this) {
									event_proc(e = event::loutdrag);
								}
							}
						}
						if(gr.lholded == this && msg == motion) {
							event_proc(e = event::ldrag);
						}
					//}
					if(gr.lholded != this) drag_origin = vec2<int>(rc.l, rc.t);
				}
			}

			rect::appearance rect::get_appearance(rect::event m) {
				if(		m == rect::event::hout 
					||	m == rect::event::lup 
					||	m == rect::event::loutup)
					return appearance::released;

				if(		m == rect::event::hover)
					return appearance::hovered;

				if(		m == rect::event::lpressed 
					||	m == rect::event::ldown 
					||	m == rect::event::ldoubleclick 
					||	m == rect::event::ltripleclick )
					return appearance::pushed;

				return appearance::unknown;
			}

			bool rect::is_scroll_aligned() {
				return rects::wh<float>(rc).is_sticked(content_size, scroll);
			}
			
			void rect::align_scroll() {
				rects::wh<float>(rc).stick_relative(content_size, scroll);
			}

			void rect::scroll_to_view() {
				if(parent) {
					rects::ltrb<float> global = get_rect_absolute();
					rects::ltrb<float> parent_global = parent->get_rect_absolute();
					vec2<int> off1 = vec2<int>(std::max(0.f, global.r + 2 - parent_global.r), std::max(0.f, global.b + 2 - parent_global.b));
					vec2<int> off2 = vec2<int>(std::max(0.f, parent_global.l - global.l + 2 + off1.x), std::max(0.f, parent_global.t - global.t + 2 + off1.y));
					parent->scroll += off1;
					parent->scroll -= off2;
					parent->scroll_to_view();
				}
			}

			rects::ltrb<float> rect::local_add(const material& mat, const rects::ltrb<float>& origin, std::vector<quad>& v) const {
				return add_quad(mat, origin+get_absolute_xy()-scroll, this, v);
			}
			
			rects::ltrb<float> rect::add_quad(const material& mat, const rects::ltrb<float>& origin, const rect* p, std::vector<quad>& v) {
				/* if p is null, we don't clip at all
				   if p is not null and p->clip is true, we take p->rc_clipped as clipper
				   if p is not null and p->clip is false, we search for the first clipping parent's rc_clipped
				*/

				return gui::add_quad(mat, origin, &p->get_clipping_rect(), v); 
			}
			
			void rect::gen_focus_links() {
				auto children_all = children;
				get_member_children(children_all);
				if(children_all.empty()) {
					//if(next) focus_next = next;
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
				sort(order.begin(), order.end(), [](rect* a, rect* b){ 
					auto& r1 = a->rc;
					auto& r2 = b->rc;
					return (r1.t == r2.t) ? (r1.l < r2.l) : (r1.t < r2.t);
				});

				//if(next == nullptr) next = this;
 					//next = focusable ? this : order[0];


				focus_next = order[0];
				focus_prev = *order.rbegin();

				for(unsigned i = 0; i < order.size(); ++i) {
					order[i]->focus_next = (i == order.size() - 1) ? this : order[i+1];
					order[i]->focus_prev = (i == 0               ) ? this : order[i-1];
				}
			}

			void rect::gen_focus_links_depth(rect* next) {
				auto children_all = children;
				get_member_children(children_all);

				if(next == nullptr)
					next = this;

				if(children_all.empty()) {
					focus_next = next;
				}

				auto order = children_all;

				/* operations on order, sort by vertical distance */
				sort(order.begin(), order.end(), [](rect* a, rect* b){ 
					auto& r1 = a->rc;
					auto& r2 = b->rc;
					return (r1.t == r2.t) ? (r1.l < r2.l) : (r1.t < r2.t);
				});

				if(!children_all.empty()) 
					focus_next = order[0];

				for(unsigned i = 0; i < order.size(); ++i) {
					order[i]->gen_focus_links_depth((i == order.size() - 1) ? next : order[i+1]);
				}
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
				
			rect* rect::get_parent() const {
				return parent;
			}
				
			rects::ltrb<float> rect::get_rect_absolute() const {
				return rects::xywh<float>(absolute_xy.x, absolute_xy.y, rc.w(), rc.h());
			}

			const vec2<int>& rect::get_absolute_xy() const {
				return absolute_xy;
			}

		}
	}
}
