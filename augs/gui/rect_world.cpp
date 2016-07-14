#pragma once
#include "rect_world.h"
#include "rect.h"
#include "window_framework/platform_utils.h"
#include "log.h"
#include "misc/object_pool.h"
#undef max
namespace augs {
	namespace gui {
		rect_world::rect_world() {
			auto new_rect = rects.allocate();
			auto& r = new_rect.get();

			r.clip = false;
			r.focusable = false;
			r.scrollable = false;

			root = new_rect;
		}

		void rect_world::set_focus(rect_id f) {
			if (f == rect_in_focus) return;
			root.calculate_clipped_rectangle_layout();

			if (rect_in_focus)
				rect_in_focus->consume_gui_event(rect::event_info(*this, rect::gui_event::blur));

			rect_in_focus = f;
			if (f) {
				f->consume_gui_event(rect::event_info(*this, rect::gui_event::focus));
				//f->scroll_to_view();
			}
		}

		rect_id rect_world::get_rect_in_focus() const {
			return rect_in_focus;
		}

		vertex_triangle_buffer rect_world::draw_triangles() const {
			vertex_triangle_buffer buffer;
			rect::draw_info in(*this, buffer);

			root.draw_children(in);

			middlescroll.draw_triangles(rects, in);

			return buffer;
		}

		void rect_world::perform_logic_step(fixed_delta delta, rect::logic_behaviour, rect::content_size_behaviour) {
			root.perform_logic_step(*this);
			root.calculate_clipped_rectangle_layout();

			middlescroll.perform_logic_step(rects, delta);

			was_hovered_rect_visited = false;

			rect::poll_info mousemotion_updater(*this, window::event::mousemotion);
			root.consume_raw_input_and_generate_gui_events(mousemotion_updater);

			if (!was_hovered_rect_visited && rect_hovered != nullptr) {
				rect_hovered->unhover(mousemotion_updater);
			}
		}


		void rect_world::set_delta_milliseconds(float delta) {
			delta_ms = delta;
		}

		float rect_world::delta_milliseconds() {
			return delta_ms;
		}

		clipboard rect_world::global_clipboard;

		void rect_world::consume_raw_input_and_generate_gui_events(augs::window::event::state new_state) {
			state = new_state;
			auto& gl = state;
			using namespace augs::window;
			rect::poll_info in(*this, gl.msg);
			bool pass = true;

			was_hovered_rect_visited = false;

			if (middlescroll.consume_raw_event(rects, new_state))
				return;

			rect::event_info e(*this, rect::gui_event::unknown);

			if (gl.msg == event::lup) {
				if (rect_held_by_lmb) {
					if (rect_held_by_lmb->get_clipped_rect().hover(gl.mouse.pos)) {
						rect_held_by_lmb->consume_gui_event(e = rect::gui_event::lup);
						rect_held_by_lmb->consume_gui_event(e = rect::gui_event::lclick);
						pass = false;
					}
					else
						rect_held_by_lmb->consume_gui_event(e = rect::gui_event::loutup);

					if (held_rect_is_dragged) 
						rect_held_by_lmb->consume_gui_event(e = rect::gui_event::lfinisheddrag);

					current_drag_amount.set(0, 0);
					rect_held_by_lmb = nullptr;
					held_rect_is_dragged = false;
				}
			}

			if (gl.msg == event::rup) {
				if (rect_held_by_rmb) {
					if (rect_held_by_rmb->get_clipped_rect().hover(gl.mouse.pos)) {
						rect_held_by_rmb->consume_gui_event(e = rect::gui_event::rup);
						rect_held_by_rmb->consume_gui_event(e = rect::gui_event::rclick);
						pass = false;
					}
					else
						rect_held_by_rmb->consume_gui_event(e = rect::gui_event::routup);
					rect_held_by_rmb = 0;
				}
			}

			if (rect_in_focus && rect_in_focus->fetch_wheel && gl.msg == event::wheel) {
				if (rect_in_focus->enable_drawing) rect_in_focus->consume_raw_input_and_generate_gui_events(in);
				pass = false;
			}
			/*
							if(gl.msg == down && gl.key == event::keys::TAB) {
								rect_id f;
								if(f = seek_focusable(focus ? focus : &root, gl.keys[event::keys::LSHIFT]))
									set_focus(f);

								pass = false;
							}*/
			if (rect_in_focus) {
				switch (gl.msg) {
				case event::keydown:   if (rect_in_focus->enable_drawing) rect_in_focus->consume_gui_event(e = rect::gui_event::keydown); pass = false; break;
				case event::keyup:	   if (rect_in_focus->enable_drawing) rect_in_focus->consume_gui_event(e = rect::gui_event::keyup); pass = false; break;
				case event::character: if (rect_in_focus->enable_drawing) rect_in_focus->consume_gui_event(e = rect::gui_event::character); pass = false; break;
				default: break;
				}
			}

			if (gl.msg == event::clipboard_change) {
				global_clipboard.change_clipboard();
				pass = false;
			}

			if (pass) {
				root.consume_raw_input_and_generate_gui_events(in);
				
				if (!was_hovered_rect_visited && rect_hovered != nullptr) {
					rect_hovered->unhover(in);
				}
			}
		}
	}
}