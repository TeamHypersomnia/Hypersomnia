#pragma once
#include <vector>
#include "augs/math/vec2.h"
#include "augs/window_framework/event.h"
#include "augs/misc/timer.h"
#include "augs/graphics/pixel.h"
#include "augs/texture_baker/texture_baker.h"
#include "augs/texture_baker/font.h"
#include "rect.h"

#include "augs/misc/pool.h"
#include "augs/misc/easier_handle_getters_mixin.h"

#include "game/assets/font_id.h"

#include "clipboard.h"
#include "middlescrolling.h"

#include "augs/misc/delta.h"
#include "gui_event.h"
#include "draw_and_event_infos.h"

namespace augs {
	namespace gui {
		class rect_world {
		public:
			static clipboard global_clipboard;

			fixed_delta delta;

			gui_element_id rect_in_focus;

			middlescrolling middlescroll;
			
			window::event::state state;

			bool was_hovered_rect_visited = false;
			bool held_rect_is_dragged = false;
			gui_element_id rect_hovered;
			gui_element_id rect_held_by_lmb;
			gui_element_id rect_held_by_rmb;
			
			vec2i ldrag_relative_anchor;
			vec2i last_ldown_position;
			vec2i current_drag_amount;

			gui_element_id root;

			template<class C>
			void set_focus(C context, const gui_element_id new_to_focus) {
				if (new_to_focus == rect_in_focus) return;

				if (context.alive(rect_in_focus)) {
					context(rect_in_focus, [this, &context](auto& r) {r.consume_gui_event(context, event_info(*this, gui_event::blur)); });
				}

				rect_in_focus = new_to_focus;

				if (context.alive(new_to_focus)) {
					context(new_to_focus, [this, &context](auto& r)  {r.consume_gui_event(context, event_info(*this, gui_event::focus)); });
				}
			}

			template<class C>
			void consume_raw_input_and_generate_gui_events(C context, const window::event::state new_state) {
				state = new_state;
				auto& gl = state;
				using namespace augs::window;
				raw_event_info in(*this, gl.msg);
				bool pass = true;

				was_hovered_rect_visited = false;

				if (middlescroll.handle_new_raw_state(context, new_state))
					return;

				auto gui_event_lambda = [this, &context](const gui_element_id& id, const gui_event ev) {
					context(id, [ev, &context](auto& r) {
						r.consume_gui_event(context, gui::event_info(*this, ev));
					});
				};

				auto consume_raw_input_lambda = [&in, &context](const gui_element_id& id) {
					context(id, [&in, &context](auto& r) {
						r.consume_raw_input_and_generate_gui_events(context, in);
					});
				};

				if (gl.msg == event::message::lup) {
					if (context.alive(rect_held_by_lmb)) {
						if (context(rect_held_by_lmb, [](const auto& r) { return r.get_clipped_rect().hover(gl.mouse.pos); })) {
							gui_event_lambda(rect_held_by_lmb, gui_event::lup);
							gui_event_lambda(rect_held_by_lmb, gui_event::lclick);
							pass = false;
						}
						else {
							gui_event_lambda(rect_held_by_lmb, gui_event::loutup);
						}

						if (held_rect_is_dragged) {
							gui_event_lambda(rect_held_by_lmb, gui_event::lfinisheddrag);
						}

						current_drag_amount.set(0, 0);
						rect_held_by_lmb = gui_element_id();
						held_rect_is_dragged = false;
					}
				}

				if (gl.msg == event::message::rup) {
					if (context.alive(rect_held_by_rmb)) {
						if (context(rect_held_by_rmb, [](const auto& r) { return r.get_clipped_rect().hover(gl.mouse.pos); })) {
							gui_event_lambda(rect_held_by_rmb, gui_event::rup);
							gui_event_lambda(rect_held_by_rmb, gui_event::rclick);
							pass = false;
						}
						else {
							gui_event_lambda(rect_held_by_rmb, gui_event::routup);
						}

						current_drag_amount.set(0, 0);
						rect_held_by_rmb = gui_element_id();
					}
				}

				if (context.alive(rect_in_focus) && rect_in_focus.get().get_flag(rect_leaf::flag::FETCH_WHEEL) && gl.msg == event::wheel) {
					if (rect_in_focus.get().get_flag(rect_leaf::flag::ENABLE_DRAWING)) {
						consume_raw_input_lambda(rect_in_focus);
					}

					pass = false;
				}
				/*
				if(gl.msg == down && gl.key == event::keys::TAB) {
				gui_element_id f;
				if(f = seek_focusable(focus ? focus : &root, gl.keys[event::keys::LSHIFT]))
				set_focus(f);

				pass = false;
				}*/
				if (rect_in_focus.alive()) {
					switch (gl.msg) {
					case event::keydown:   if (rect_in_focus.get().get_flag(rect_leaf::flag::ENABLE_DRAWING)) rect_in_focus.consume_gui_event(e = gui_event::keydown, dispatcher); pass = false; break;
					case event::keyup:	   if (rect_in_focus.get().get_flag(rect_leaf::flag::ENABLE_DRAWING)) rect_in_focus.consume_gui_event(e = gui_event::keyup, dispatcher); pass = false; break;
					case event::character: if (rect_in_focus.get().get_flag(rect_leaf::flag::ENABLE_DRAWING)) rect_in_focus.consume_gui_event(e = gui_event::character, dispatcher); pass = false; break;
					default: break;
					}
				}

				if (gl.msg == event::clipboard_change) {
					global_clipboard.change_clipboard();
					pass = false;
				}

				if (pass) {
					consume_raw_input_lambda(root);

					if (!was_hovered_rect_visited && rects[rect_hovered].alive()) {
						rects[rect_hovered].unhover(in, dispatcher);}
				}
			}

			template<class C>
			void perform_logic_step(C context) {
				auto root_handle = rects[root];

				root_handle.perform_logic_step(*this, dispatcher);
				root_handle.calculate_clipped_rectangle_layout(dispatcher);

				middlescroll.perform_logic_step(rects, delta, state);

				was_hovered_rect_visited = false;

				raw_event_info mousemotion_updater(*this, window::event::mousemotion);
				root_handle.consume_raw_input_and_generate_gui_events(mousemotion_updater, dispatcher);

				auto rect_hovered_handle = rects[rect_hovered];

				if (!was_hovered_rect_visited && rect_hovered_handle.alive()) {
					rect_hovered_handle.unhover(mousemotion_updater, dispatcher);
				}
			}
			
			template<class C>
			vertex_triangle_buffer draw_triangles(C context) const {
				vertex_triangle_buffer buffer;
				draw_info in(*this, buffer);

				rects[root].draw_children(in, dispatcher);

				middlescroll.draw_triangles(context, in);

				return buffer;
			}

			gui_element_id get_rect_in_focus() const;
		};
	}
}

