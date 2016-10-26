#pragma once
#include <vector>
#include <bitset>
#include "material.h"
#include "gui_element_id.h"
#include "draw_and_event_infos.h"
#include "augs/window_framework/event.h"
#include "augs/misc/delta.h"
#include "gui_flags.h"

namespace augs {
	namespace gui {
		struct stylesheet;

		template <class derived, class gui_element_id>
		struct rect_leaf {
			std::bitset<static_cast<size_t>(flag::COUNT)> flags;
			vec2i rc_pos_before_dragging;

			rects::ltrb<float> rc; /* actual rectangle */

			rect_leaf(const rects::xywh<float>& rc = rects::xywh<float>()) : rc(rc) {
				set_default_flags();
			}

			rect_leaf(const assets::texture_id& id) {
				set_default_flags();
				rc.set_size((*id).get_size());
			}

			void set_default_flags() {
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

			bool get_flag(const flag f) const {
				return flags.test(static_cast<size_t>(f));
			}

			bool set_flag(const flag f) {
				flags.set(static_cast<size_t>(f));
			}

			bool unset_flag(const flag f) {
				flags.set(static_cast<size_t>(f), false);
			}

			vec2 get_scroll() const {
				return vec2();
			}

			void set_scroll(const vec2) {

			}

			template<class L>
			decltype(auto) this_call(L lambda) {
				derived* self = static_cast<derived*>(self);
				return lambda(*self);
			}

			template<class L>
			decltype(auto) this_call(L lambda) const {
				const derived* self = static_cast<const derived*>(self);
				return lambda(*self);
			}

			template<class C>
			void build_tree_data(C context, const gui_element_id& this_id) const {
				auto& tree_entry = context.get_tree_entry(this_id);
				const auto parent = tree_entry.get_parent();
				
				auto absolute_clipped_rect = rc;
				auto absolute_pos = vec2i(rc.l, rc.t);
				auto absolute_clipping_rect = rects::ltrb<float>(0.f, 0.f, std::numeric_limits<int>::max() / 2.f, std::numeric_limits<int>::max() / 2.f);

				/* if we have parent */
				if (context.alive(parent)) {
					context(parent, [&](const auto& parent_rect) {
						auto& p = context.get_tree_entry(parent);
						const auto scroll = parent_rect.get_scroll();

						/* we have to save our global coordinates in absolute_xy */
						absolute_pos = p.get_absolute_pos() + vec2i(rc.l, rc.t) - vec2i(int(scroll.x), int(scroll.y));
						absolute_clipped_rect = rects::xywh<float>(absolute_pos.x, absolute_pos.y, rc.w(), rc.h());

						/* and we have to clip by first clipping parent's rc_clipped */
						//auto* clipping = get_clipping_parent(); 
						//if(clipping) 
						absolute_clipped_rect.clip_by(p.get_absolute_clipping_rect());

						absolute_clipping_rect = p.get_absolute_clipping_rect();
					});
				}

				if (get_flag(flag::CLIP))
					absolute_clipping_rect.clip_by(absolute_clipped_rect);

				/* align scroll only to be positive and not to exceed content size */
				//if (get_flag(flag::SNAP_SCROLL_TO_CONTENT_SIZE)) {
				//	this_call([](auto& r) {
				//		r.clamp_scroll_to_right_down_corner();
				//	});
				//}

				tree_entry.set_absolute_clipped_rect(absolute_clipped_rect);
				tree_entry.set_absolute_pos(absolute_pos);
				tree_entry.set_absolute_clipping_rect(absolute_clipping_rect);

				this_call([&](auto& r) {
					r.for_each_child([&](auto& r, const gui_element_id& child_id) {
						context.get_tree_entry(child_id).set_parent(this_id);
						r.build_tree_data(context, child_id);
					})
				});
			}

			template<class C>
			void consume_raw_input_and_generate_gui_events(C context, const gui_element_id& this_id, gui::raw_event_info& inf) {
				using namespace augs::window::event;
				auto& gr = context.get_rect_world();
				auto& tree_entry = context.get_tree_entry(this_id);
				const auto& state = inf.state;
				const auto& m = state.mouse;
				const auto& msg = inf.state.msg;

				auto gui_event_lambda = [&](const gui_event ev) {
					this_call([&](auto& r) {
						r.consume_gui_event(context, this_id, ev);
					});
				};

				if (get_flag(flag::ENABLE_DRAWING)) {
					if (get_flag(flag::ENABLE_DRAWING_OF_CHILDREN)) {
						this_call([&](auto& r) {
							r.for_each_child(context, [&](auto& r, const gui_element_id& child_id) {
								if (r.get_flag(flag::ENABLE_DRAWING)) {
									r.consume_raw_input_and_generate_gui_events(context, child_id, inf);
								}
							});
						});
					}
				}

				if (get_flag(flag::ENABLE_DRAWING)) {
					if (!get_flag(flag::DISABLE_HOVERING)) {
						const auto absolute_clipped_rect = tree_entry.get_absolute_clipped_rect();
						const bool hover = absolute_clipped_rect.good() && absolute_clipped_rect.hover(m.pos);

						if (context.dead(gr.rect_hovered)) {
							if (hover) {
								gui_event_lambda(gui_event::hover);
								gr.rect_hovered = this_id;
								gr.was_hovered_rect_visited = true;
							}
						}
						else if (gr.rect_hovered == this_id) {
							const bool still_hover = hover;

							if (still_hover) {
								if (msg == message::lup) {
									gui_event_lambda(gui_event::lup);
								}
								if (msg == message::ldown) {
									gr.rect_held_by_lmb = this_id;
									gr.ldrag_relative_anchor = m.pos - rc.get_position();
									gr.last_ldown_position = m.pos;
									rc_pos_before_dragging = vec2i(rc.l, rc.t);
									gui_event_lambda(gui_event::ldown);
								}
								if (msg == message::mdown) {
									gui_event_lambda(gui_event::mdown);
								}
								if (msg == message::mdoubleclick) {
									gui_event_lambda(gui_event::mdoubleclick);
								}
								if (msg == message::ldoubleclick) {
									gr.rect_held_by_lmb = this_id;
									gr.ldrag_relative_anchor = m.pos - rc.get_position();
									gr.last_ldown_position = m.pos;
									gui_event_lambda(gui_event::ldoubleclick);
								}
								if (msg == message::ltripleclick) {
									gr.rect_held_by_lmb = this_id;
									gr.ldrag_relative_anchor = m.pos - rc.get_position();
									gr.last_ldown_position = m.pos;
									gui_event_lambda(gui_event::ltripleclick);
								}
								if (msg == message::rdown) {
									gr.rect_held_by_rmb = this_id;
									gui_event_lambda(gui_event::rdown);
								}
								if (msg == message::rdoubleclick) {
									gr.rect_held_by_rmb = this_id;
									gui_event_lambda(gui_event::rdoubleclick);
								}

								if (msg == message::wheel) {
									gui_event_lambda(gui_event::wheel);
								}

								if (gr.rect_held_by_lmb == this_id && msg == message::mousemotion && state.mouse_keys[0] && absolute_clipped_rect.hover(m.ldrag)) {
									gui_event_lambda(gui_event::lpressed);
								}
								if (gr.rect_held_by_rmb == this_id && msg == message::mousemotion && state.mouse_keys.state[1] && absolute_clipped_rect.hover(m.rdrag)) {
									gui_event_lambda(gui_event::rpressed);
								}
							}
							else {
								// ensure(msg == message::mousemotion);
								gui_event_lambda(gui_event::hout);
								unhover(context, this_id, inf);
							}

							gr.was_hovered_rect_visited = true;
						}
					}

					if (gr.rect_held_by_lmb == this_id && msg == message::mousemotion && m.pos != gr.last_ldown_position) {
						gr.held_rect_is_dragged = true;
						gr.current_drag_amount = m.pos - gr.last_ldown_position;
						gui_event_lambda(gui_event::ldrag);
					}
				}
			}

			template<class C>
			void consume_gui_event(C context, const gui_element_id& this_id, const event_info e) const {
				// try_to_make_this_rect_focused(context, this_id, e);
				//	scroll_content_with_wheel(context, e);
				//	try_to_enable_middlescrolling(context, e);
			}

			template<class C>
			void perform_logic_step(C context, const gui_element_id& this_id, const fixed_delta& delta) {
				if (!get_flag(flag::ENABLE_DRAWING_OF_CHILDREN)) {
					return;
				}

				perform_logic_step_on_children(context, this_id, delta);
			}

			template<class C>
			void perform_logic_step_on_children(C context, const gui_element_id& this_id, const fixed_delta& delta) {
				this_call([&](const auto& r) {
					r.for_each_child(context, [&](auto& r, const gui_element_id& child_id) {
						if (r.get_flag(flag::ENABLE_DRAWING)) {
							r.perform_logic_step(context, child_id, delta);
						}
					});
				});
			}

			template<class C>
			void draw(C context, draw_info in) const {
				if (!get_flag(flag::ENABLE_DRAWING_OF_CHILDREN)) {
					return;
				}

				draw_children(context, in);
			}

			template <class C>
			void draw_children(C context, draw_info in) const {
				this_call([&](const auto& r) {
					r.for_each_child(context, [&](const auto& r, const gui_element_id& id) {
						if (r.get_flag(flag::ENABLE_DRAWING)) {
							r.draw(context, in);
							r.draw_children(context, in);
						}
					});
				});
			}

			template<class C>
			static void draw_stretched_texture(C context, const gui_element_id& id, gui::draw_info in, const gui::material& mat = gui::material()) {
				const auto absolute = context.get_tree_entry(id).get_absolute_rect();
				draw_clipped_rectangle(mat, absolute, context.get_tree_entry(id).get_parent(), in.v).good();
			}

			template<class C>
			static void draw_centered_texture(C context, const gui_element_id& id, gui::draw_info in, const gui::material& = gui::material(), const vec2i offset = vec2i()) {
				auto absolute_centered = context.get_tree_entry(id).get_absolute_rect();
				const auto tex_size = (*mat.tex).get_size();
				absolute_centered.l += absolute_centered.w() / 2 - float(tex_size.x) / 2;
				absolute_centered.t += absolute_centered.h() / 2 - float(tex_size.y) / 2;
				absolute_centered.l = int(absolute_centered.l) + offset.x;
				absolute_centered.t = int(absolute_centered.t) + offset.y;
				absolute_centered.w(tex_size.x);
				absolute_centered.h(tex_size.y);

				draw_clipped_rectangle(mat, absolute_centered, context, context.get_tree_entry(id).get_parent(), in.v).good();
			}

			template<class C>
			static void draw_rectangle_stylesheeted(C context, const gui_element_id& id, gui::draw_info in, const gui::stylesheet& styles) {
				const auto st = styles.get_style();
				const auto& this_entry = context.get_tree_entry(id);

				if (st.color.active || st.background_image.active) {
					draw_stretched_texture(context, in, material(st));
				}

				if (st.border.active) {
					st.border.value.draw(in.v, this_entry.get_absolute_rect(), context, context.get_tree_entry(this_entry.get_parent()).get_absolute_clipping_rect());
				}
			}

			template<class C, class L>
			void for_each_child(C context, L lambda) {

			}

			template <class C>
			void unhover(C context, const gui_element_id& this_id, gui::raw_event_info& inf) {
				auto& world = context.get_rect_world();

				auto gui_event_lambda = [&](const gui_event ev) {
					this_call([&](auto& r) {
						r.consume_gui_event(context, this_id, ev);
					});
				};

				gui_event_lambda(gui_event::hoverlost);

				if (world.rect_held_by_lmb == this_id)
					gui_event_lambda(gui_event::loutdrag);

				world.rect_hovered = gui_element_id();
			}

			//template<class C>
			//void try_to_make_this_rect_focused(C context, gui_element_id this_id, const gui::event_info e) {
			//	if (!get_flag(flag::FOCUSABLE)) return;
			//	
			//	auto& sys = context.get_rect_world();
			//
			//	if (e == gui_event::ldown ||
			//		e == gui_event::ldoubleclick ||
			//		e == gui_event::ltripleclick ||
			//		e == gui_event::rdoubleclick ||
			//		e == gui_event::rdown
			//		) {
			//		if (context.alive(sys.get_rect_in_focus())) {
			//			if (get_flag(flag::PRESERVE_FOCUS) || !context(sys.get_rect_in_focus(), [](const auto& r) { return r.get_flag(flag::PRESERVE_FOCUS); }))
			//				sys.set_focus(this_id);
			//		}
			//		else sys.set_focus(this_id);
			//	}
			//}

		};

			//bool is_scroll_clamped_to_right_down_corner() const;
			//void clamp_scroll_to_right_down_corner();

			//template<class C>
			//rects::wh<float> calculate_content_size(C context) const {
			//	/* init on zero */
			//	rects::ltrb<float> content = rects::ltrb<float>(0.f, 0.f, 0.f, 0.f);
			//
			//	/* enlarge the content size by every child */
			//	context(this_id, [&content](const auto& r) {
			//		r.for_each_child(context, [&content](const auto& r) {
			//			if (r.get_flag(flag::ENABLE_DRAWING)) {
			//				content.contain_positive(r.rc);
			//			}
			//		});
			//	});
			//
			//	return content;
			//}

			//template<class C>
			//void scroll_content_with_wheel(C context, const gui::event_info e) {
			//	auto& owner = e.owner;
			//	const auto& wnd = owner.state;
			//	const bool scrollable = get_flag(flag::SCROLLABLE);
			//
			//	const bool parent_alive = context.alive(parent);
			//	
			//	auto call_parent_wheel_callback = [this, &context, &owner]() {
			//		context(parent, [&owner, &context](auto& p) { p.consume_gui_event(context, gui::event_info(owner, gui_event::wheel)); });
			//	};
			//
			//	if (e == gui_event::wheel) {
			//		if (wnd.keys[augs::window::event::keys::SHIFT]) {
			//			int temp = static_cast<int>(scroll.x);
			//			if (scrollable) {
			//
			//				scroll.x -= wnd.mouse.scroll;
			//				clamp_scroll_to_right_down_corner();
			//			}
			//			if ((!scrollable || temp == scroll.x) && parent_alive) {
			//				call_parent_wheel_callback();
			//			}
			//		}
			//		else {
			//			int temp = static_cast<int>(scroll.x);
			//
			//			if (scrollable) {
			//				scroll.y -= wnd.mouse.scroll;
			//				clamp_scroll_to_right_down_corner();
			//			}
			//			if ((!scrollable || temp == scroll.y) && parent_alive) {
			//				call_parent_wheel_callback();
			//			}
			//		}
			//	}
			//}
			//
			//template<class C>
			//void try_to_enable_middlescrolling(C context, const gui::event_info e) {
			//	auto& owner = e.owner;
			//	auto& wnd = owner.state;
			//
			//	if (e == gui_event::mdown || e == gui_event::mdoubleclick) {
			//		if (get_flag(flag::SCROLLABLE) && !content_size.inside(rects::wh<float>(rc))) {
			//			owner.middlescroll.subject = this_id;
			//			owner.middlescroll.pos = wnd.mouse.pos;
			//			owner.set_focus(this_id);
			//		}
			//		else if (context.alive(parent)) {
			//			context(parent, [&context, e](auto& p) { p.consume_gui_event(context, e); })
			//		}
			//	}
			//}
			//
			//template<class C>
			//void scroll_to_view(C context) {
			//	if (context.alive(parent)) {
			//		context(parent, [&](const auto& p) {
			//			const rects::ltrb<float> global = get_absolute_rect();
			//			const rects::ltrb<float> parent_global = p.get_absolute_rect();
			//			const vec2i off1 = vec2i(std::max(0.f, global.r + 2 - parent_global.r), std::max(0.f, global.b + 2 - parent_global.b));
			//			const vec2i off2 = vec2i(std::max(0.f, parent_global.l - global.l + 2 + off1.x), std::max(0.f, parent_global.t - global.t + 2 + off1.y));
			//			p.scroll += off1;
			//			p.scroll -= off2;
			//
			//			p.scroll_to_view(context);
			//		});
			//	}
			//}
	}
}