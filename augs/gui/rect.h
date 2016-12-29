#pragma once
#include <vector>
#include <bitset>
#include "material.h"
#include "gui_traversal_structs.h"
#include "augs/window_framework/event.h"
#include "augs/misc/delta.h"
#include "augs/misc/enum_bitset.h"
#include "gui_flags.h"

namespace augs {
	namespace gui {
		struct stylesheet;

		struct rect_node_data {
			augs::enum_bitset<flag> flags;
			vec2i rc_pos_before_dragging;

			rects::ltrb<float> rc; /* actual rectangle */

			rect_node_data(const rects::xywh<float>& rc = rects::xywh<float>());
			rect_node_data(const assets::texture_id& id);

			void set_default_flags();

			void set_flag(const flag f, const bool value = true);
			void unset_flag(const flag f);
			
			bool get_flag(const flag f) const;

			void set_scroll(const vec2);
			vec2 get_scroll() const;
		};

		template <class gui_element_polymorphic_id>
		struct rect_node : rect_node_data {
			typedef augs::gui::gui_entropy<gui_element_polymorphic_id> gui_entropy;

			using rect_node_data::rect_node_data;

			template <class C, class gui_element_id>
			static void build_tree_data(C context, const gui_element_id& this_id) {
				auto& tree_entry = context.make_tree_entry(this_id);
				const auto parent = tree_entry.get_parent();
				
				auto absolute_clipped_rect = this_id->rc;
				auto absolute_pos = vec2i(this_id->rc.get_position());
				auto absolute_clipping_rect = rects::ltrb<float>(0.f, 0.f, std::numeric_limits<int>::max() / 2.f, std::numeric_limits<int>::max() / 2.f);

				/* if we have parent */
				if (context.alive(parent)) {
					context(parent, [&](const auto& parent_rect) {
						auto& p = context.get_tree_entry(parent);
						const auto scroll = parent_rect->get_scroll();

						/* we have to save our global coordinates in absolute_xy */
						absolute_pos = p.get_absolute_pos() + this_id->rc.get_position() - scroll;
						absolute_clipped_rect = rects::xywh<float>(static_cast<float>(absolute_pos.x), static_cast<float>(absolute_pos.y), this_id->rc.w(), this_id->rc.h());

						/* and we have to clip by first clipping parent's rc_clipped */
						//auto* clipping = get_clipping_parent(); 
						//if(clipping) 
						absolute_clipped_rect.clip_by(p.get_absolute_clipping_rect());

						absolute_clipping_rect = p.get_absolute_clipping_rect();
					});
				}

				if (this_id->get_flag(flag::CLIP)) {
					absolute_clipping_rect.clip_by(absolute_clipped_rect);
				}

				/* align scroll only to be positive and not to exceed content size */
				//if (get_flag(flag::SNAP_SCROLL_TO_CONTENT_SIZE)) {
				//	this_call([](const auto& r) {
				//		r->clamp_scroll_to_right_down_corner();
				//	});
				//}

				tree_entry.set_absolute_clipped_rect(absolute_clipped_rect);
				tree_entry.set_absolute_pos(absolute_pos);
				tree_entry.set_absolute_clipping_rect(absolute_clipping_rect);

				this_id->for_each_child(context, this_id, [&](const auto& child_id) {
					context.make_tree_entry(child_id).set_parent(this_id);
					child_id->build_tree_data(context, child_id);
				});
			}

			template <class C, class gui_element_id>
			static void consume_raw_input_and_generate_gui_events(C context, const gui_element_id& this_id, gui::event_traversal_flags& inf, gui_entropy& entropies) {
				using namespace augs::window::event;
				auto& gr = context.get_rect_world();
				const auto& tree_entry = context.get_tree_entry(this_id);
				const auto& state = gr.last_state;
				const auto& m = state.mouse;
				const auto& msg = inf.change.msg;
				const auto& mouse_pos = m.pos;

				auto gui_event_lambda = [&](const gui_event ev, const int scroll_amount = 0) {
					entropies.post_event(this_id, { ev, scroll_amount });
				};

				if (this_id->get_flag(flag::ENABLE_DRAWING)) {
					if (this_id->get_flag(flag::ENABLE_DRAWING_OF_CHILDREN)) {
						this_id->for_each_child(context, this_id, [&](const auto& child_id) {
							if (child_id->get_flag(flag::ENABLE_DRAWING)) {
								child_id->consume_raw_input_and_generate_gui_events(context, child_id, inf, entropies);
							}
						});
					}

					if (!this_id->get_flag(flag::DISABLE_HOVERING)) {
						const auto absolute_clipped_rect = tree_entry.get_absolute_clipped_rect();
						const bool hover = absolute_clipped_rect.good() && absolute_clipped_rect.hover(mouse_pos);

						if (context.dead(gr.rect_hovered)) {
							if (hover) {
								gui_event_lambda(gui_event::hover);
								gr.rect_hovered.set(this_id);
								inf.was_hovered_rect_visited = true;
							}
						}
						else if (gr.rect_hovered == this_id) {
							const bool still_hover = hover;

							if (still_hover) {
								if (msg == message::lup) {
									gui_event_lambda(gui_event::lup);
								}
								if (msg == message::ldown) {
									gr.rect_held_by_lmb.set(this_id);
									gr.ldrag_relative_anchor = mouse_pos - this_id->rc.get_position();
									gr.last_ldown_position = mouse_pos;
									this_id->rc_pos_before_dragging = vec2i(this_id->rc.get_position());
									gui_event_lambda(gui_event::ldown);
								}
								if (msg == message::mdown) {
									gui_event_lambda(gui_event::mdown);
								}
								if (msg == message::mdoubleclick) {
									gui_event_lambda(gui_event::mdoubleclick);
								}
								if (msg == message::ldoubleclick) {
									gr.rect_held_by_lmb.set(this_id);
									gr.ldrag_relative_anchor = mouse_pos - this_id->rc.get_position();
									gr.last_ldown_position = mouse_pos;
									gui_event_lambda(gui_event::ldoubleclick);
								}
								if (msg == message::ltripleclick) {
									gr.rect_held_by_lmb.set(this_id);
									gr.ldrag_relative_anchor = mouse_pos - this_id->rc.get_position();
									gr.last_ldown_position = mouse_pos;
									gui_event_lambda(gui_event::ltripleclick);
								}
								if (msg == message::rdown) {
									gr.rect_held_by_rmb.set(this_id);
									gui_event_lambda(gui_event::rdown);
								}
								if (msg == message::rdoubleclick) {
									gr.rect_held_by_rmb.set(this_id);
									gui_event_lambda(gui_event::rdoubleclick);
								}

								if (msg == message::wheel) {
									gui_event_lambda(gui_event::wheel, inf.change.scroll.amount);
								}

								if (gr.rect_held_by_lmb == this_id && msg == message::mousemotion && state.get_mouse_key(0) && absolute_clipped_rect.hover(m.ldrag)) {
									gui_event_lambda(gui_event::lpressed);
								}
								if (gr.rect_held_by_rmb == this_id && msg == message::mousemotion && state.get_mouse_key(1) && absolute_clipped_rect.hover(m.rdrag)) {
									gui_event_lambda(gui_event::rpressed);
								}
							}
							else {
								// ensure(msg == message::mousemotion);
								gui_event_lambda(gui_event::hout);
								unhover(context, this_id, inf, entropies);
							}

							inf.was_hovered_rect_visited = true;
						}
					}

					if (gr.rect_held_by_lmb == this_id && msg == message::mousemotion && mouse_pos != gr.last_ldown_position) {
						gr.held_rect_is_dragged = true;
						gr.current_drag_amount = mouse_pos - gr.last_ldown_position;
						gui_event_lambda(gui_event::ldrag);
					}
				}
			}

			//template <class C, class gui_element_id>
			//static void consume_gui_event(C context, const gui_element_id& this_id, const event_info e) {
				// try_to_make_this_rect_focused(context, this_id, e);
				//	scroll_content_with_wheel(context, e);
				//	try_to_enable_middlescrolling(context, e);
			//}

			template <class C, class gui_element_id>
			static void advance_elements(C context, const gui_element_id& this_id, const gui_entropy& entropies) {
				this_id->for_each_child(context, this_id, [&](const auto& child_id) {
					child_id->advance_elements(context, child_id, entropies);
				});
			}

			template <class C, class gui_element_id>
			static void rebuild_layouts(C context, const gui_element_id& this_id) {
				this_id->for_each_child(context, this_id, [&](const auto& child_id) {
					child_id->rebuild_layouts(context, child_id);
				});
			}
			
			template <class C, class gui_element_id>
			static void draw(C context, const gui_element_id& this_id, draw_info in) {
				if (!this_id->get_flag(flag::ENABLE_DRAWING)) {
					return;
				}

				draw_children(context, this_id, in);
			}

			template <class C, class gui_element_id>
			static void draw_children(C context, const gui_element_id& this_id, draw_info in) {
				if (!this_id->get_flag(flag::ENABLE_DRAWING_OF_CHILDREN)) {
					return;
				}

				this_id->for_each_child(context, this_id, [&](const auto& child_id) {
					child_id->draw(context, child_id, in);
				});
			}

			template <class C, class gui_element_id>
			static void draw_stretched_texture(C context, const gui_element_id& id, gui::draw_info in, const gui::material& mat = gui::material()) {
				const auto absolute = context.get_tree_entry(id).get_absolute_rect();
				draw_clipped_rectangle(mat, absolute, context, context.get_tree_entry(id).get_parent(), in.v);
			}

			template <class C, class gui_element_id>
			static void draw_centered_texture(C context, const gui_element_id& id, gui::draw_info in, const gui::material& mat = gui::material(), const vec2i offset = vec2i()) {
				auto absolute_centered = context.get_tree_entry(id).get_absolute_rect();
				const auto tex_size = (*mat.tex).get_size();
				absolute_centered.l += absolute_centered.w() / 2 - float(tex_size.x) / 2;
				absolute_centered.t += absolute_centered.h() / 2 - float(tex_size.y) / 2;
				absolute_centered.l = float(int(absolute_centered.l) + offset.x);
				absolute_centered.t = float(int(absolute_centered.t) + offset.y);
				absolute_centered.w(float(tex_size.x));
				absolute_centered.h(float(tex_size.y));

				draw_clipped_rectangle(mat, absolute_centered, context, context.get_tree_entry(id).get_parent(), in.v);
			}

			template <class C, class gui_element_id>
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

			template <class C, class gui_element_id, class L>
			static void for_each_child(C&, const gui_element_id&, L) {

			}

			template <class C, class gui_element_id>
			static void unhover(C context, const gui_element_id& this_id, gui::event_traversal_flags& inf, gui_entropy& entropies) {
				auto& world = context.get_rect_world();

				auto gui_event_lambda = [&](const gui_event ev) {
					entropies.post_event(this_id, ev);
				};

				gui_event_lambda(gui_event::hoverlost);

				if (world.rect_held_by_lmb == this_id) {
					gui_event_lambda(gui_event::loutdrag);
				}

				world.rect_hovered.unset();
			}

			//template <class C, class gui_element_id>
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
			//			if (get_flag(flag::PRESERVE_FOCUS) || !context(sys.get_rect_in_focus(), [](const auto& r) { return r->get_flag(flag::PRESERVE_FOCUS); }))
			//				sys.set_focus(this_id);
			//		}
			//		else sys.set_focus(this_id);
			//	}
			//}

		};

			//bool is_scroll_clamped_to_right_down_corner() const;
			//void clamp_scroll_to_right_down_corner();

			//template <class C, class gui_element_id>
			//rects::wh<float> calculate_content_size(C context) const {
			//	/* init on zero */
			//	rects::ltrb<float> content = rects::ltrb<float>(0.f, 0.f, 0.f, 0.f);
			//
			//	/* enlarge the content size by every child */
			//	context(this_id, [&content](const auto& r) {
			//		r->for_each_child(context, [&content](const auto& r) {
			//			if (r->get_flag(flag::ENABLE_DRAWING)) {
			//				content.contain_positive(r->rc);
			//			}
			//		});
			//	});
			//
			//	return content;
			//}

			//template <class C, class gui_element_id>
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
			//		if (wnd.keys[augs::window::event::keys::key::SHIFT]) {
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
			//template <class C, class gui_element_id>
			//void try_to_enable_middlescrolling(C context, const gui::event_info e) {
			//	auto& owner = e.owner;
			//	auto& wnd = owner.state;
			//
			//	if (e == gui_event::mdown || e == gui_event::mdoubleclick) {
			//		if (get_flag(flag::SCROLLABLE) && !content_size.inside(rects::wh<float>(rc))) {
			//			owner.middlescroll.subject.set(this_id);
			//			owner.middlescroll.pos = wnd.mouse.pos;
			//			owner.set_focus(this_id);
			//		}
			//		else if (context.alive(parent)) {
			//			context(parent, [&context, e](auto& p) { p.consume_gui_event(context, e); })
			//		}
			//	}
			//}
			//
			//template <class C, class gui_element_id>
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