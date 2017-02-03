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

		template <class gui_element_polymorphic_id>
		class rect_world {
		public:
			typedef augs::gui::gui_entropy<gui_element_polymorphic_id> gui_entropy;

			template<class C, class gui_element_id>
			bool is_hovered(const C context, const gui_element_id& id) {
				return context(id, [&](const auto& p) {
					return context.get_tree_entry(id).get_absolute_clipped_rect().hover(last_state.mouse.pos);
				});
			}

			window::event::state last_state;

			middlescrolling<gui_element_polymorphic_id> middlescroll;
			
			bool held_rect_is_dragged = false;
			padding_byte pad[3];
			
			gui_element_polymorphic_id rect_hovered;
			gui_element_polymorphic_id rect_held_by_lmb;
			gui_element_polymorphic_id rect_held_by_rmb;

			gui_element_polymorphic_id rect_in_focus;
			
			vec2i ldrag_relative_anchor;
			vec2i last_ldown_position;
			vec2i current_drag_amount;

			template <class gui_element_id>
			bool is_currently_dragging(const gui_element_id& id) const {
				return rect_held_by_lmb == id && held_rect_is_dragged;
			}

			bool is_currently_dragging() const {
				return held_rect_is_dragged;
			}

			gui_element_polymorphic_id get_rect_in_focus() const {
				return rect_in_focus;
			}

			template <class gui_element_id>
			static void generate_gui_event(gui_entropy& entropies, const gui_element_id& id, event_info in) {
				entropies.post_event(id, in);
			}

			template <class C, class gui_element_id>
			void set_focus(const C context, const gui_element_id& new_to_focus) {
				if (new_to_focus == rect_in_focus) {
					return;
				}

				if (context.alive(rect_in_focus)) {
					generate_gui_event(entropies, rect_in_focus, gui_event::blur);
				}

				rect_in_focus = new_to_focus;

				if (context.alive(new_to_focus)) {
					generate_gui_event(entropies, new_to_focus, gui_event::focus);
				}
			}

			template <class C, class gui_element_id>
			void consume_raw_input_and_generate_gui_events(const C context, const gui_element_id& root, const window::event::change new_state, gui_entropy& entropies) {
				using namespace augs::window;

				last_state.apply(new_state);

				event_traversal_flags in(new_state);
				bool pass = true;

				if (middlescroll.handle_new_raw_state(context, new_state)) {
					return;
				}

				if (new_state.msg == event::message::lup) {
					if (context.alive(rect_held_by_lmb)) {
						if (is_hovered(context, rect_held_by_lmb)) {
							generate_gui_event(entropies, rect_held_by_lmb, gui_event::lup);
							generate_gui_event(entropies, rect_held_by_lmb, gui_event::lclick);
							pass = false;
						}
						else {
							generate_gui_event(entropies, rect_held_by_lmb, gui_event::loutup);
						}

						if (held_rect_is_dragged) {
							generate_gui_event(entropies, rect_held_by_lmb, { gui_event::lfinisheddrag, 0, current_drag_amount } );
						}

						current_drag_amount.set(0, 0);
						rect_held_by_lmb.unset();
						held_rect_is_dragged = false;
					}
				}

				if (new_state.msg == event::message::rup) {
					if (context.alive(rect_held_by_rmb)) {
						if (is_hovered(context, rect_held_by_rmb)) {
							generate_gui_event(entropies, rect_held_by_rmb, gui_event::rup);
							generate_gui_event(entropies, rect_held_by_rmb, gui_event::rclick);
							pass = false;
						}
						else {
							generate_gui_event(entropies, rect_held_by_rmb, gui_event::routup);
						}

						current_drag_amount.set(0, 0);
						rect_held_by_rmb.unset();
					}
				}

				if (context.alive(rect_in_focus) 
					&& context(rect_in_focus, [](const auto& r) { return r->get_flag(flag::FETCH_WHEEL); } ) 
					&& new_state.msg == event::message::wheel) {

					if (context(rect_in_focus, [](const auto& r) { return r->get_flag(flag::ENABLE_DRAWING); })) {
						context(rect_in_focus, [&](const auto& r) {
							r->consume_raw_input_and_generate_gui_events(context, r, in, entropies);
						});
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
					const bool rect_in_focus_drawing_enabled = context(rect_in_focus, [](const auto& r) { return r->get_flag(flag::ENABLE_DRAWING); });

					switch (new_state.msg) {
					case event::message::keydown:   
						if (rect_in_focus_drawing_enabled) {
							generate_gui_event(entropies, rect_in_focus, gui_event::keydown);
						}

						pass = false;
						break;
					case event::message::keyup:	    
						if (rect_in_focus_drawing_enabled) {
							generate_gui_event(entropies, rect_in_focus, gui_event::keyup);
						}

						pass = false;
						break;
					case event::message::character: 
						if (rect_in_focus_drawing_enabled) {
							generate_gui_event(entropies, rect_in_focus, gui_event::character);
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
					context(root, [&](const auto& r) {
						r->consume_raw_input_and_generate_gui_events(context, r, in, entropies);
					});

					if (!in.was_hovered_rect_visited && context.alive(rect_hovered)) {
						context(rect_hovered, [&](const auto& r) { 
							r->unhover(context, r, in, entropies);
						});
					}
				}
			}

			template <class C, class gui_element_id>
			void build_tree_data_into_context(const C context, const gui_element_id& root) const {
				context(root, [&](const auto& r) { 
					r->build_tree_data(context, r); 
				});
			}

			template <class C, class gui_element_id>
			void advance_elements(
				const C context, 
				const gui_element_id& root, 
				const augs::delta dt
			) {
				context(root, [&](const auto& r) {
					r->advance_elements(context, r, dt);
				});

				middlescroll.advance_elements(context, dt);
			}

			template <class C, class gui_element_id>
			void respond_to_events(
				const C context, 
				const gui_element_id& root, 
				const gui_entropy& entropies
			) {
				context(root, [&](const auto& r) {
					r->respond_to_events(
						context, 
						r, 
						entropies
					);
				});
			}
			
			template <class C, class gui_element_id>
			void rebuild_layouts(const C context, const gui_element_id& root) {
				context(root, [&](const auto& r) {
					r->rebuild_layouts(context, r);
				});
			}
			
			template <class C, class gui_element_id>
			void call_idle_mousemotion_updater(const C context, const gui_element_id& root, gui_entropy& entropy) {
				window::event::change fabricated_state;
				fabricated_state.msg = window::event::message::mousemotion;
				fabricated_state.mouse.rel.set(0, 0);

				event_traversal_flags mousemotion_updater(fabricated_state);

				context(root, [&](const auto& r) {
					r->consume_raw_input_and_generate_gui_events(context, r, mousemotion_updater, entropy);
				});

				if (!mousemotion_updater.was_hovered_rect_visited && context.alive(rect_hovered)) {
					context(rect_hovered, [&](const auto& r) { r->unhover(context, r, mousemotion_updater, entropy); });
				}
			}

			template <class C, class gui_element_id>
			void draw(vertex_triangle_buffer& output_buffer, const C context, const gui_element_id& root) const {
				draw_info in(output_buffer);

				context(root, [&](const auto& r) { 
					r->draw_children(context, r, in);
				});

				middlescroll.draw(context, in);
			}
		};
	}
}

