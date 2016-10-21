#pragma once
#include <vector>
#include <bitset>
#include "material.h"
#include "gui_element_id.h"
#include "draw_and_event_infos.h"
#include "augs/window_framework/event.h"
#include "augs/misc/delta.h"

namespace augs {
	namespace gui {
		struct stylesheet;
		class rect_world;

		struct rect_leaf {
			enum class flag {
				DISABLE_HOVERING,
				ENABLE_DRAWING,
				FETCH_WHEEL,
				PRESERVE_FOCUS,
				FOCUSABLE,
				ENABLE_DRAWING_OF_CHILDREN,
				SNAP_SCROLL_TO_CONTENT_SIZE,
				SCROLLABLE,
				CLIP,

				COUNT
			};

			std::bitset<static_cast<size_t>(flag::COUNT)> flags;
			vec2i rc_pos_before_dragging;

			rects::ltrb<float> rc; /* actual rectangle */

			rect_leaf(const gui_element_id& this_id, const rects::xywh<float>& rc = rects::xywh<float>());
			rect_leaf(const gui_element_id& this_id, const assets::texture_id&);

			const rects::ltrb<float>& get_clipped_rect() const;
			rects::ltrb<float> get_rect_absolute() const;
			const vec2i& get_absolute_xy() const;

			gui_element_id this_id;
			gui_element_id parent;

			rects::ltrb<float> rc_clipped;

			vec2i absolute_xy;

			void set_default_flags();

			bool get_flag(const flag) const;
			bool set_flag(const flag);
			bool unset_flag(const flag);

			rects::ltrb<float> get_clipping_rect() const;
			vec2 get_scroll() const;
			void set_scroll(const vec2);

			template<class C>
			void calculate_clipped_rectangle_layout(C context) {
				rc_clipped = rc;
				absolute_xy = vec2i(rc.l, rc.t);

				/* if we have parent */
				if (context.alive(parent)) {
					context(parent, [this](const auto& p) {
						/* we have to save our global coordinates in absolute_xy */
						absolute_xy = p.absolute_xy + vec2i(rc.l, rc.t) - vec2i(int(p.get_scroll().x), int(p.get_scroll().y));
						rc_clipped = rects::xywh<float>(absolute_xy.x, absolute_xy.y, rc.w(), rc.h());

						/* and we have to clip by first clipping parent's rc_clipped */
						//auto* clipping = get_clipping_parent(); 
						//if(clipping) 
						rc_clipped.clip_by(p.get_clipping_rect());

						clipping_rect = p.get_clipping_rect();
					});
				}
			}

			template<class C>
			void consume_raw_input_and_generate_gui_events(C context, gui::raw_event_info& inf) {
				using namespace augs::window::event;
				auto& gr = inf.owner;
				const auto& m = gr.state.mouse;
				const auto msg = inf.msg;

				auto gui_event_lambda = [this, &context, &gr](const gui_event ev) {
					context(this_id, [ev, &context, &gr](auto& r) {
						r.consume_gui_event(context, gui::event_info(gr, ev));
					});
				};

				if (get_flag(rect_leaf::flag::ENABLE_DRAWING)) {
					if (!get_flag(rect_leaf::flag::DISABLE_HOVERING)) {
						if (gr.rect_hovered == nullptr) {
							const bool hover = rc_clipped.good() && rc_clipped.hover(m.pos);

							if (hover) {
								gui_event_lambda(gui_event::hover);
								gr.rect_hovered = this_id;
								gr.was_hovered_rect_visited = true;
							}
						}
						else if (gr.rect_hovered == this_id) {
							const bool still_hover = rc_clipped.good() && rc_clipped.hover(m.pos);

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

								if (gr.rect_held_by_lmb == this_id && msg == message::mousemotion && m.state[0] && rc_clipped.hover(m.ldrag)) {
									gui_event_lambda(gui_event::lpressed);
								}
								if (gr.rect_held_by_rmb == this_id && msg == message::mousemotion && m.state[1] && rc_clipped.hover(m.rdrag)) {
									gui_event_lambda(gui_event::rpressed);
								}
							}
							else {
								// ensure(msg == message::mousemotion);
								gui_event_lambda(gui_event::hout);
								unhover(inf);
							}

							gr.was_hovered_rect_visited = true;
						}
					}

					if (gr.rect_held_by_lmb == this && msg == message::mousemotion && m.pos != gr.last_ldown_position) {
						gr.held_rect_is_dragged = true;
						gr.current_drag_amount = m.pos - gr.last_ldown_position;
						gui_event_lambda(gui_event::ldrag);
					}
				}
			}

			template<class C>
			void consume_gui_event(C context, event_info e) const {
				try_to_make_this_rect_focused(context, e);
			}

			template<class C>
			void perform_logic_step(C context, const fixed_delta& delta) {

			}

			template<class C>
			void recalculate_content_size(C) {

			}

			template<class C>
			void draw(C, draw_info) const {

			}

			template<class C>
			void draw_stretched_texture(C context, gui::draw_info in, const gui::material& mat = gui::material()) const {
				draw_clipped_rectangle(mat, get_rect_absolute(), context(parent, [](const auto& p) { return static_cast<rect_leaf&>(p); }), in.v).good();
			}

			template<class C>
			void draw_centered_texture(C context, gui::draw_info in, const gui::material& = gui::material(), const vec2i offset = vec2i()) const {
				auto absolute_centered = this->get().get_rect_absolute();
				const auto tex_size = (*mat.tex).get_size();
				absolute_centered.l += absolute_centered.w() / 2 - float(tex_size.x) / 2;
				absolute_centered.t += absolute_centered.h() / 2 - float(tex_size.y) / 2;
				absolute_centered.l = int(absolute_centered.l) + offset.x;
				absolute_centered.t = int(absolute_centered.t) + offset.y;
				absolute_centered.w(tex_size.x);
				absolute_centered.h(tex_size.y);

				draw_clipped_rectangle(mat, absolute_centered, context(parent, [](const auto& p) { return static_cast<rect_leaf&>(p); }), in.v).good();
			}

			template<class C>
			void draw_rectangle_stylesheeted(C context, gui::draw_info in, const gui::stylesheet& styles) const {
					auto st = styles.get_style();

					if (st.color.active || st.background_image.active)
						draw_stretched_texture(context, in, material(st));

					if (st.border.active) st.border.value.draw(in.v, get_rect_absolute(), context(parent, [](const auto& p) { return p.get_clipping_rect(); }));
			}

			template<class C, class L>
			void for_each_child(C context, L lambda) {

			}

			template<class C>
			void try_to_make_this_rect_focused(C context, const gui::event_info e) {
				if (!get_flag(flag::FOCUSABLE)) return;
				
				auto& sys = e.owner;

				if (e == gui_event::ldown ||
					e == gui_event::ldoubleclick ||
					e == gui_event::ltripleclick ||
					e == gui_event::rdoubleclick ||
					e == gui_event::rdown
					) {
					if (context.alive(sys.get_rect_in_focus())) {
						if (get_flag(flag::PRESERVE_FOCUS) || !context(sys.get_rect_in_focus(), [](const auto& r) { return r.get_flag(flag::PRESERVE_FOCUS); }))
							sys.set_focus(this_id);
					}
					else sys.set_focus(this_id);
				}
			}

			template <class C>
			void unhover(C context, gui::raw_event_info& inf) const {
				auto gui_event_lambda = [this, &context, &inf](const gui_event ev) {
					context(this_id, [ev, &context, &inf](auto& r) {
						r.consume_gui_event(context, gui::event_info(inf.owner, ev));
					});
				};

				gui_event_lambda(gui_event::hoverlost);

				if (inf.owner.rect_held_by_lmb == this_id)
					gui_event_lambda(gui_event::loutdrag);

				inf.owner.rect_hovered = gui_element_id();
			}

			bool is_being_dragged(const gui::rect_world&) const;
		};

		struct rect_composite : rect_leaf {
			rects::wh<float> content_size; /* content's (children's) bounding box */
			vec2 scroll; /* scrolls content */

			rects::ltrb<float> clipping_rect = rects::ltrb<float>(0.f, 0.f, std::numeric_limits<int>::max() / 2.f, std::numeric_limits<int>::max() / 2.f);

			rects::ltrb<float> get_clipping_rect() const;
			rects::ltrb<float> get_local_clipper() const;

			/*  does scroll not exceed the content */
			bool is_scroll_clamped_to_right_down_corner() const;

			/* align scroll to not exceed the content */
			void clamp_scroll_to_right_down_corner();
			
			vec2 get_scroll() const;
			void set_scroll(const vec2);

			template<class C>
			void calculate_clipped_rectangle_layout(C context) {
				rect_leaf::calculate_clipped_rectangle_layout(context);

				if (get_flag(flag::CLIP))
					clipping_rect.clip_by(rc_clipped);

				context(this_id, [](auto& r) {
					r.recalculate_context_size(context);
				});

				/* align scroll only to be positive and not to exceed content size */
				if (get_flag(flag::SNAP_SCROLL_TO_CONTENT_SIZE))
					clamp_scroll_to_right_down_corner();

				context(this_id, [this, &context](auto& r) {
					r.for_each_child([this, &context](auto& r) {
						r.parent = this_id;
						r.calculate_clipped_rectangle_layout(context);
					})
				});
			}

			template<class C>
			void consume_raw_input_and_generate_gui_events(C context, gui::raw_event_info& inf) {
				using namespace augs::window::event;
				auto& gr = inf.owner;
				auto& m = gr.state.mouse;
				auto msg = inf.msg;

				if (get_flag(rect_leaf::flag::ENABLE_DRAWING)) {
					if (get_flag(flag::ENABLE_DRAWING_OF_CHILDREN)) {
						context(this_id, [this, &inf, &context](auto& r) {
							r.for_each_child(context, [this, &inf, &context](auto& r) {
								if (r.get_flag(rect_leaf::flag::ENABLE_DRAWING)) {
									r.parent = this_id;
									r.consume_raw_input_and_generate_gui_events(context, inf);
								}
							});
						});
					}
				}

				rect_leaf::consume_raw_input_and_generate_gui_events(context, inf);
			}

			template<class C>
			void consume_gui_event(C context, event_info e) {
				rect_leaf::gui_event_lambda(e);
				scroll_content_with_wheel(context, e);
				try_to_enable_middlescrolling(context, e);
			}

			template<class C>
			void perform_logic_step(C context, const fixed_delta& delta) {
				context(this_id, [this, &delta, &context](auto& r) {
					r.for_each_child(context, [this, &delta, &context](auto& r) {
						if (r.get_flag(rect_leaf::flag::ENABLE_DRAWING)) {
							r.parent = this_id;
							r.perform_logic_step(context, delta);
						}
					});
				});
			}

			template<class C>
			void draw(C context, draw_info in) const {

			}

			template <class C>
			void draw_children(C context, draw_info in) const {
				if (!get_flag(flag::ENABLE_DRAWING_OF_CHILDREN))
					return;

				context(this_id, [this, &in, &context](const auto& r) {
					r.for_each_child(context, [this, &in, &context](const auto& r) {
						if (r.get_flag(rect_leaf::flag::ENABLE_DRAWING)) {
							r.draw(context, in);
							r.draw_children(context, in);
						}
					});
				});
			}

			template<class C>
			void recalculate_content_size(C context) const {
				/* init on zero */
				rects::ltrb<float> content = rects::ltrb<float>(0.f, 0.f, 0.f, 0.f);

				/* enlarge the content size by every child */
				context(this_id, [&content](const auto& r) {
					r.for_each_child(context, [&content](const auto& r) {
						if (r.get_flag(rect_leaf::flag::ENABLE_DRAWING)) {
							content.contain_positive(r.rc);
						}
					});
				});

				content_size = content;
			}

			template<class C>
			void scroll_content_with_wheel(C context, const gui::event_info e) {
				auto& owner = e.owner;
				const auto& wnd = owner.state;
				const bool scrollable = get_flag(flag::SCROLLABLE);

				const bool parent_alive = context.alive(parent);
				
				auto call_parent_wheel_callback = [this, &context, &owner]() {
					context(parent, [&owner, &context](auto& p) { p.consume_gui_event(context, gui::event_info(owner, gui_event::wheel)); });
				};

				if (e == gui_event::wheel) {
					if (wnd.keys[augs::window::event::keys::SHIFT]) {
						int temp = static_cast<int>(scroll.x);

						if (scrollable) {
							scroll.x -= wnd.mouse.scroll;
							clamp_scroll_to_right_down_corner();
						}
						if ((!scrollable || temp == scroll.x) && parent_alive) {
							call_parent_wheel_callback();
						}
					}
					else {
						int temp = static_cast<int>(scroll.x);

						if (scrollable) {
							scroll.y -= wnd.mouse.scroll;
							clamp_scroll_to_right_down_corner();
						}
						if ((!scrollable || temp == scroll.y) && parent_alive) {
							call_parent_wheel_callback();
						}
					}
				}
			}

			template<class C>
			void try_to_enable_middlescrolling(C context, const gui::event_info e) {
				auto& owner = e.owner;
				auto& wnd = owner.state;

				if (e == gui_event::mdown || e == gui_event::mdoubleclick) {
					if (get_flag(flag::SCROLLABLE) && !content_size.inside(rects::wh<float>(rc))) {
						owner.middlescroll.subject = this_id;
						owner.middlescroll.pos = wnd.mouse.pos;
						owner.set_focus(this_id);
					}
					else if (context.alive(parent)) {
						context(parent, [&context, e](auto& p) { p.consume_gui_event(context, e); })
					}
				}
			}

			template<class C>
			void scroll_to_view(C context) {
				if (context.alive(parent)) {
					context(parent, [this, &context](const auto& p) {
						const rects::ltrb<float> global = get_rect_absolute();
						const rects::ltrb<float> parent_global = p.get_rect_absolute();
						const vec2i off1 = vec2i(std::max(0.f, global.r + 2 - parent_global.r), std::max(0.f, global.b + 2 - parent_global.b));
						const vec2i off2 = vec2i(std::max(0.f, parent_global.l - global.l + 2 + off1.x), std::max(0.f, parent_global.t - global.t + 2 + off1.y));
						p.scroll += off1;
						p.scroll -= off2;

						p.scroll_to_view(context);
					});
				}
			}
		};

		struct rect_composite_vector : rect_composite {
			std::vector<gui_element_id> children;

			template<class C, class L>
			void for_each_child(C context, L lambda) {
				const auto& children_all = children;

				for (const auto& c : children) {
					context(c, lambda);
				}
			}
		};
	}
}