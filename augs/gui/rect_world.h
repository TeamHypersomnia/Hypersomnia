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
#include "gui_traversal_structs.h"
#include "augs/padding_byte.h"

namespace augs {
	namespace gui {
		extern clipboard global_clipboard;

		template<class C, class gui_element_id>
		void polymorphic_consume_gui_event(C context, const gui_element_id& id, const gui_event ev) {
			context(id, [&](auto& r) {
				r.consume_gui_event(context, id, ev);
			});
		}

		template <class gui_element_id>
		class rect_world {
		public:
			window::event::state last_state;

			middlescrolling<gui_element_id> middlescroll;
			
			bool held_rect_is_dragged = false;
			padding_byte pad[3];
			
			gui_element_id root;

			gui_element_id rect_hovered;
			gui_element_id rect_held_by_lmb;
			gui_element_id rect_held_by_rmb;

			gui_element_id rect_in_focus;
			
			vec2i ldrag_relative_anchor;
			vec2i last_ldown_position;
			vec2i current_drag_amount;
			vec2i last_mouse_pos;

			bool is_being_dragged(const gui_element_id& id) const {
				return rect_held_by_lmb == id && held_rect_is_dragged;
			}

			gui_element_id get_rect_in_focus() const {
				return rect_in_focus;
			}

			template<class C>
			void set_focus(C context, const gui_element_id new_to_focus) {
				if (new_to_focus == rect_in_focus) {
					return;
				}

				if (context.alive(rect_in_focus)) {
					polymorphic_consume_gui_event(rect_in_focus, gui_event::blur);
				}

				rect_in_focus = new_to_focus;

				if (context.alive(new_to_focus)) {
					polymorphic_consume_gui_event(new_to_focus, gui_event::focus);
				}
			}

			template<class C>
			void consume_raw_input_and_generate_gui_events(C context, const window::event::change new_state) {
				using namespace augs::window;

				last_state.apply(new_state);

				event_traversal_flags in(new_state);
				bool pass = true;

				if (middlescroll.handle_new_raw_state(context, new_state))
					return;

				auto consume_raw_input_lambda = [&](const gui_element_id& id) {
					context(id, [&](auto& r) {
						r.consume_raw_input_and_generate_gui_events(context, id, in);
					});
				};

				auto is_hovered_lambda = [&](const gui_element_id& id) {
					return context(id, [&](const auto& p) {
						return context.get_tree_entry(id).get_absolute_clipped_rect().hover(new_state.mouse.pos);
					});
				};

				if (new_state.msg == event::message::lup) {
					if (context.alive(rect_held_by_lmb)) {
						if (is_hovered_lambda(rect_held_by_lmb)) {
							polymorphic_consume_gui_event(rect_held_by_lmb, gui_event::lup);
							polymorphic_consume_gui_event(rect_held_by_lmb, gui_event::lclick);
							pass = false;
						}
						else {
							polymorphic_consume_gui_event(rect_held_by_lmb, gui_event::loutup);
						}

						if (held_rect_is_dragged) {
							polymorphic_consume_gui_event(rect_held_by_lmb, gui_event::lfinisheddrag);
						}

						current_drag_amount.set(0, 0);
						rect_held_by_lmb = gui_element_id();
						held_rect_is_dragged = false;
					}
				}

				if (new_state.msg == event::message::rup) {
					if (context.alive(rect_held_by_rmb)) {
						if (is_hovered_lambda(rect_held_by_rmb)) {
							polymorphic_consume_gui_event(rect_held_by_rmb, gui_event::rup);
							polymorphic_consume_gui_event(rect_held_by_rmb, gui_event::rclick);
							pass = false;
						}
						else {
							polymorphic_consume_gui_event(rect_held_by_rmb, gui_event::routup);
						}

						current_drag_amount.set(0, 0);
						rect_held_by_rmb = gui_element_id();
					}
				}

				if (context.alive(rect_in_focus) 
					&& context(rect_in_focus, [](const auto& r) { return r.get_flag(flag::FETCH_WHEEL); } ) 
					&& new_state.msg == event::message::wheel) {

					if (context(rect_in_focus, [](const auto& r) { return r.get_flag(flag::ENABLE_DRAWING); })) {
						consume_raw_input_lambda(rect_in_focus);
					}

					pass = false;
				}
				/*
				if(new_state.msg == down && new_state.key == event::keys::key::TAB) {
				gui_element_id f;
				if(f = seek_focusable(focus ? focus : &root, new_state.keys[event::keys::key::LSHIFT]))
				set_focus(f);

				pass = false;
				}*/
				if (context.alive(rect_in_focus)) {
					const bool rect_in_focus_drawing_enabled = context(rect_in_focus, [](const auto& r) { return r.get_flag(flag::ENABLE_DRAWING); });

					switch (new_state.msg) {
					case event::message::keydown:   
						if (rect_in_focus_drawing_enabled) {
							polymorphic_consume_gui_event(rect_in_focus, gui_event::keydown);
						}

						pass = false;
						break;
					case event::message::keyup:	    
						if (rect_in_focus_drawing_enabled) {
							polymorphic_consume_gui_event(rect_in_focus, gui_event::keyup);
						}

						pass = false;
						break;
					case event::message::character: 
						if (rect_in_focus_drawing_enabled) {
							polymorphic_consume_gui_event(rect_in_focus, gui_event::character);
						}

						pass = false;
						break;
					default: break;
					}
				}

				if (new_state.msg == event::message::clipboard_change) {
					global_clipboard.change_clipboard();
					pass = false;
				}

				if (pass) {
					consume_raw_input_lambda(root);

					if (!in.was_hovered_rect_visited && context.alive(rect_hovered)) {
						context(rect_hovered, [&](auto& r) { 
							r.unhover(context, rect_hovered, in);
						});
					}
				}
			}

			template <class C>
			void build_tree_data_into_context(C context) const {
				context(root, [&](const auto& r) { 
					r.build_tree_data(context, root); 
				});
			}

			template<class C>
			void perform_logic_step(C context, const fixed_delta& delta) {
				context(root, [&](auto& r) { 
					r.perform_logic_step(context, root, delta);
				});

				middlescroll.perform_logic_step(context, delta);
			}
			
			template<class C>
			void call_idle_mousemotion_updater(C context) {
				window::event::change fabricated_state;
				fabricated_state.msg = window::event::message::mousemotion;
				fabricated_state.mouse.rel.set(0, 0);

				event_traversal_flags mousemotion_updater(fabricated_state);

				context(root, [&](auto& r) {
					r.consume_raw_input_and_generate_gui_events(context, root, mousemotion_updater);
				});

				if (!mousemotion_updater.was_hovered_rect_visited && context.alive(rect_hovered)) {
					context(rect_hovered, [&](auto& r) { r.unhover(context, rect_hovered, mousemotion_updater); });
				}
			}

			template<class C>
			vertex_triangle_buffer draw_triangles(C context) const {
				vertex_triangle_buffer buffer;
				draw_info in(*this, buffer);

				context(root, [&context, &in](auto& r) { r.draw_children(context, in); });
				middlescroll.draw_triangles(context, in);

				return buffer;
			}
		};
	}
}

