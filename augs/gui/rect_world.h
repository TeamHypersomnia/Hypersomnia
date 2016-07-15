#pragma once
#include <vector>
#include <functional>
#include "math/vec2.h"
#include "window_framework/event.h"
#include "misc/timer.h"
#include "graphics/pixel.h"
#include "texture_baker/texture_baker.h"
#include "texture_baker/font.h"
#include "rect.h"

#include "misc/pool.h"
#include "misc/pool_handlizer.h"

#include "game/assets/font_id.h"

#include "clipboard.h"
#include "middlescrolling.h"

#include "misc/delta.h"
#include "gui_event.h"

namespace augs {
	namespace gui {
		class rect_world {
		public:
			static clipboard global_clipboard;

			fixed_delta delta;

			rect_id rect_in_focus;

			middlescrolling middlescroll;
			
			window::event::state state;

			bool was_hovered_rect_visited = false;
			bool held_rect_is_dragged = false;
			rect_id rect_hovered;
			rect_id rect_held_by_lmb;
			rect_id rect_held_by_rmb;
			
			vec2i ldrag_relative_anchor;
			vec2i last_ldown_position;
			vec2i current_drag_amount;

			rect_id root;

			template<class D>
			void set_focus(rect_handle, D dispatcher) {
				if (f == rect_in_focus) return;
				auto& rects = f.get_pool();

				auto in_focus = rects[rect_in_focus];

				if (in_focus.alive())
					in_focus.consume_gui_event(event_info(*this, gui_event::blur), dispatcher);

				rect_in_focus = f;

				if (f.alive()) {
					f.consume_gui_event(event_info(*this, gui_event::focus), dispatcher);
				}
			}

			template<class D>
			void consume_raw_input_and_generate_gui_events(rect_pool&, window::event::state, D dispatcher) {
				state = new_state;
				auto& gl = state;
				using namespace augs::window;
				raw_event_info in(*this, gl.msg);
				bool pass = true;

				was_hovered_rect_visited = false;

				if (middlescroll.handle_new_raw_state(rects, new_state))
					return;

				event_info e(*this, gui_event::unknown);

				auto rect_held_by_lmb = rects[this->rect_held_by_lmb];

				if (gl.msg == event::lup) {
					if (rect_held_by_lmb.alive()) {
						if (rect_held_by_lmb.get().get_clipped_rect().hover(gl.mouse.pos)) {
							rect_held_by_lmb.consume_gui_event(e = gui_event::lup, dispatcher);
							rect_held_by_lmb.consume_gui_event(e = gui_event::lclick, dispatcher);
							pass = false;
						}
						else
							rect_held_by_lmb.consume_gui_event(e = gui_event::loutup, dispatcher);

						if (held_rect_is_dragged)
							rect_held_by_lmb.consume_gui_event(e = gui_event::lfinisheddrag, dispatcher);

						current_drag_amount.set(0, 0);
						this->rect_held_by_lmb = rect_id();
						held_rect_is_dragged = false;
					}
				}

				auto rect_held_by_rmb = rects[this->rect_held_by_rmb];

				if (gl.msg == event::rup) {
					if (rect_held_by_rmb.alive()) {
						if (rect_held_by_rmb.get().get_clipped_rect().hover(gl.mouse.pos)) {
							rect_held_by_rmb.consume_gui_event(e = gui_event::rup, dispatcher);
							rect_held_by_rmb.consume_gui_event(e = gui_event::rclick, dispatcher);
							pass = false;
						}
						else
							rect_held_by_rmb.consume_gui_event(e = gui_event::routup, dispatcher);

						this->rect_held_by_rmb = rect_id();
					}
				}

				auto rect_in_focus = rects[this->rect_in_focus];

				if (rect_in_focus.alive() && rect_in_focus.get().fetch_wheel && gl.msg == event::wheel) {
					if (rect_in_focus.get().enable_drawing) rect_in_focus.consume_raw_input_and_generate_gui_events(in, dispatcher);
					pass = false;
				}
				/*
				if(gl.msg == down && gl.key == event::keys::TAB) {
				rect_id f;
				if(f = seek_focusable(focus ? focus : &root, gl.keys[event::keys::LSHIFT]))
				set_focus(f);

				pass = false;
				}*/
				if (rect_in_focus.alive()) {
					switch (gl.msg) {
					case event::keydown:   if (rect_in_focus.get().enable_drawing) rect_in_focus.consume_gui_event(e = gui_event::keydown, dispatcher); pass = false; break;
					case event::keyup:	   if (rect_in_focus.get().enable_drawing) rect_in_focus.consume_gui_event(e = gui_event::keyup, dispatcher); pass = false; break;
					case event::character: if (rect_in_focus.get().enable_drawing) rect_in_focus.consume_gui_event(e = gui_event::character, dispatcher); pass = false; break;
					default: break;
					}
				}

				if (gl.msg == event::clipboard_change) {
					global_clipboard.change_clipboard();
					pass = false;
				}

				if (pass) {
					rects[root].consume_raw_input_and_generate_gui_events(in, dispatcher);

					if (!was_hovered_rect_visited && rects[rect_hovered].alive()) {
						rects[rect_hovered].unhover(in);
					}
				}
			}

			template<class D>
			void perform_logic_step(rect_pool&, D dispatcher) {
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
			
			template<class D>
			vertex_triangle_buffer draw_triangles(const rect_pool&, D dispatcher) const {
				vertex_triangle_buffer buffer;
				draw_info in(*this, buffer);

				rects[root].draw_children(in, dispatcher);

				middlescroll.draw_triangles(rects, in);

				return buffer;
			}

			rect_id get_rect_in_focus() const;
		};

		struct draw_info {
			const rect_world& owner;
			vertex_triangle_buffer& v;

			draw_info(const rect_world&, vertex_triangle_buffer&);
		};

		struct raw_event_info {
			rect_world& owner;
			const unsigned msg;

			bool mouse_fetched = false;
			bool scroll_fetched = false;
			raw_event_info(rect_world&, unsigned);
		};

		struct event_info {
			rect_world& owner;
			gui_event msg;

			event_info(rect_world&, gui_event);
			operator gui_event();
			event_info& operator=(gui_event);
		};
	}
}

