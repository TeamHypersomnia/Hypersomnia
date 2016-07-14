#pragma once
#include "rect_world.h"
#include "rect.h"
#include "window_framework/platform_utils.h"
#include "log.h"
#include "misc/pool.h"

#include "rect_handle.h"

#undef max
namespace augs {
	namespace gui {
		clipboard rect_world::global_clipboard;

		void rect_world::set_focus(rect_handle f, event_behaviour behaviour) {
			if (f == rect_in_focus) return;
			auto& rects = f.get_pool();

			auto in_focus = rects[rect_in_focus];

			if (in_focus.alive())
				behaviour(in_focus, event_info(*this, gui_event::blur));

			rect_in_focus = f;

			if (f.alive()) {
				behaviour(f, event_info(*this, gui_event::focus));
			}
		}

		rect_id rect_world::get_rect_in_focus() const {
			return rect_in_focus;
		}

		vertex_triangle_buffer rect_world::draw_triangles(const rect_pool& rects, draw_behaviour behaviour) const {
			vertex_triangle_buffer buffer;
			draw_info in(*this, buffer);

			rects[root].get().draw_children(in);

			middlescroll.draw_triangles(rects, in);

			return buffer;
		}
		
		void rect_world::perform_logic_step(rect_pool& rects, event_behaviour events, logic_behaviour logic, content_size_behaviour content_size) {
			auto root_handle = rects[root];
			
			root_handle.perform_logic_step(*this, logic);
			root_handle.calculate_clipped_rectangle_layout(content_size);
		
			middlescroll.perform_logic_step(rects, delta, state);
		
			was_hovered_rect_visited = false;
		
			raw_event_info mousemotion_updater(*this, window::event::mousemotion);
			root_handle.consume_raw_input_and_generate_gui_events(mousemotion_updater, events);
		
			auto rect_hovered_handle = rects[rect_hovered];

			if (!was_hovered_rect_visited && rect_hovered_handle.alive()) {
				rect_hovered_handle.unhover(mousemotion_updater);
			}
		}
		
		void rect_world::consume_raw_input_and_generate_gui_events(rect_pool& rects, window::event::state new_state, event_behaviour behaviour) {
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
						rect_held_by_lmb.consume_gui_event(e = gui_event::lup, behaviour);
						rect_held_by_lmb.consume_gui_event(e = gui_event::lclick, behaviour);
						pass = false;
					}
					else
						rect_held_by_lmb.consume_gui_event(e = gui_event::loutup, behaviour);
		
					if (held_rect_is_dragged) 
						rect_held_by_lmb.consume_gui_event(e = gui_event::lfinisheddrag, behaviour);
		
					current_drag_amount.set(0, 0);
					this->rect_held_by_lmb = rect_id();
					held_rect_is_dragged = false;
				}
			}
		
			auto rect_held_by_rmb = rects[this->rect_held_by_rmb];

			if (gl.msg == event::rup) {
				if (rect_held_by_rmb.alive()) {
					if (rect_held_by_rmb.get().get_clipped_rect().hover(gl.mouse.pos)) {
						rect_held_by_rmb.consume_gui_event(e = gui_event::rup, behaviour);
						rect_held_by_rmb.consume_gui_event(e = gui_event::rclick, behaviour);
						pass = false;
					}
					else
						rect_held_by_rmb.consume_gui_event(e = gui_event::routup, behaviour);
					
					this->rect_held_by_rmb = rect_id();
				}
			}
		
			auto rect_in_focus = rects[this->rect_in_focus];

			if (rect_in_focus.alive() && rect_in_focus.get().fetch_wheel && gl.msg == event::wheel) {
				if (rect_in_focus.get().enable_drawing) rect_in_focus.consume_raw_input_and_generate_gui_events(in, behaviour);
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
				case event::keydown:   if (rect_in_focus.get().enable_drawing) rect_in_focus.consume_gui_event(e = gui_event::keydown, behaviour); pass = false; break;
				case event::keyup:	   if (rect_in_focus.get().enable_drawing) rect_in_focus.consume_gui_event(e = gui_event::keyup, behaviour); pass = false; break;
				case event::character: if (rect_in_focus.get().enable_drawing) rect_in_focus.consume_gui_event(e = gui_event::character, behaviour); pass = false; break;
				default: break;
				}
			}
		
			if (gl.msg == event::clipboard_change) {
				global_clipboard.change_clipboard();
				pass = false;
			}
		
			if (pass) {
				rects[root].consume_raw_input_and_generate_gui_events(in, behaviour);
				
				if (!was_hovered_rect_visited && rects[rect_hovered].alive()) {
					rects[rect_hovered].unhover(in);
				}
			}
		}
	}
}