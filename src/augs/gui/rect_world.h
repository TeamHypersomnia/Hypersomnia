#pragma once
#include <vector>

#include "augs/math/vec2.h"
#include "augs/window_framework/event.h"
#include "augs/graphics/rgba.h"
#include "augs/image/font.h"
#include "rect.h"

#include "augs/misc/pool/pool.h"

#include "clipboard.h"
#include "middlescrolling.h"

#include "augs/misc/timing/delta.h"
#include "gui_event.h"
#include "gui_traversal_structs.h"
#include "augs/pad_bytes.h"

namespace augs {
	namespace gui {
		extern clipboard global_clipboard;

		template <class gui_element_variant_id>
		class rect_world {
		public:
			using gui_entropy = augs::gui::gui_entropy<gui_element_variant_id>;

			middlescrolling<gui_element_variant_id> middlescroll;
			
			bool held_rect_is_dragged = false;
			pad_bytes<3> pad;
			
			gui_element_variant_id rect_hovered;
			gui_element_variant_id rect_held_by_lmb;
			gui_element_variant_id rect_held_by_rmb;

			gui_element_variant_id rect_in_focus;
			
			vec2i ldrag_relative_anchor;
			vec2i last_ldown_position;
			vec2i current_drag_amount;

			template <class C, class gui_element_id>
			bool is_hovered(const C context, const gui_element_id& id) {
				return context.get_tree_entry(id).get_absolute_clipped_rect().hover(context.get_input_state().mouse.pos);
			}

			template <class gui_element_id>
			bool is_currently_dragging(const gui_element_id& id) const {
				return rect_held_by_lmb == gui_element_variant_id(id) && held_rect_is_dragged;
			}

			bool is_currently_dragging() const {
				return held_rect_is_dragged;
			}

			template <class C>
			bool wants_to_capture_mouse(const C context) const {
				if (context.alive(rect_hovered)) {
					return true;
				}
				else {
					return is_currently_dragging();
				}
			}

			gui_element_variant_id get_rect_in_focus() const {
				return rect_in_focus;
			}

			template <class gui_element_id>
			static void generate_gui_event(gui_entropy& entropies, const gui_element_id& id, event_info in) {
				entropies.post_event(id, in);
			}

			template <class C>
			void unhover_and_undrag(const C context) {
				{
					gui_entropy entropies;

					if (context.alive(rect_held_by_lmb)) {
						generate_gui_event(entropies, rect_held_by_lmb, gui_event::loutup);

						if (held_rect_is_dragged) {
							generate_gui_event(entropies, rect_held_by_lmb, { gui_event::lfinisheddrag, 0, current_drag_amount });
							held_rect_is_dragged = false;
						}

						current_drag_amount.set(0, 0);
						rect_held_by_lmb = gui_element_variant_id();
					}

					respond_to_events(context, entropies);
				}

				{
					gui_entropy entropies;

					if (context.alive(rect_held_by_rmb)) {
						generate_gui_event(entropies, rect_held_by_rmb, gui_event::routup);

						current_drag_amount.set(0, 0);
						rect_held_by_rmb = gui_element_variant_id();
					}

					respond_to_events(context, entropies);
				}

				{
					gui_entropy entropies;

					if (context.alive(rect_hovered)) {
						context(rect_hovered, [&](const auto& r) {
							event::change ch;
							raw_input_traversal dummy(ch);

							r->unhover(context, r, dummy, entropies);
						});

						rect_hovered = gui_element_variant_id();
					}

					respond_to_events(context, entropies);
				}
			}

			template <class C>
			auto consume_raw_input_and_generate_gui_events(
				const C context,
				const event::change event
			) {
				gui_entropy entropy;
				consume_raw_input_and_generate_gui_events(context, event, entropy);
				return entropy;
			}

			template <class C>
			void consume_raw_input_and_generate_gui_events(
				const C context, 
				const event::change new_state, 
				gui_entropy& entropies
			) {
				if (context.tree.empty()) {
					return;
				}

				using namespace augs;
				const auto root = context.get_root_id();

				raw_input_traversal in(new_state);
				bool pass = true;

				if (middlescroll.handle_new_raw_state(context, new_state)) {
					return;
				}

				using namespace event::keys;

				if (new_state.was_released(key::LMOUSE)) {
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
							held_rect_is_dragged = false;
						}

						current_drag_amount.set(0, 0);
						rect_held_by_lmb = gui_element_variant_id();
					}
				}

				if (new_state.was_released(key::RMOUSE)) {
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
						rect_held_by_rmb = gui_element_variant_id();
					}
				}

				if (
					new_state.msg == event::message::wheel
					&& context.alive(rect_in_focus)
					&& context(rect_in_focus, [](const auto& r) { return r->get_flag(flag::FETCH_WHEEL); } ) 
				) {
					if (context(rect_in_focus, [](const auto& r) { return r->get_flag(flag::ENABLE_DRAWING); })) {
						context(rect_in_focus, [&](const auto& r) {
							r->consume_raw_input_and_generate_gui_events(context, r, in, entropies);
						});
					}

					pass = false;
				}
				
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
					//global_clipboard.change_clipboard();
					pass = false;
				}

				if (pass) {
					context(root, [&](const auto& r) {
						r->consume_raw_input_and_generate_gui_events(context, r, in, entropies);
					});

					if (/* hovered_but_unvisited */ context.alive(rect_hovered) && !in.was_hovered_rect_visited) {
						context(rect_hovered, [&](const auto& r) { 
							r->unhover(context, r, in, entropies);
						});
					}
				}
			}

			template <class C>
			void build_tree_data_into(const C context) const {
				context.tree.clear();

				context(context.get_root_id(), [&](const auto& r) { 
					r->build_tree_data(context, r, gui_element_variant_id());
				});
			}

			template <class C>
			void advance_elements(
				const C context,
				const augs::delta dt
			) {
				context(context.get_root_id(), [&](const auto& r) {
					r->advance_elements(context, r, dt);
				});

				middlescroll.advance_elements(context, dt);
			}

			template <class C>
			void respond_to_events(
				const C context, 
				const gui_entropy& entropies
			) {
				if (entropies.empty()) {
					return;
				}

				context(context.get_root_id(), [&](const auto& r) {
					r->respond_to_events(
						context, 
						r, 
						entropies
					);
				});
			}
			
			template <class C>
			void rebuild_layouts(const C context) {
				context(context.get_root_id(), [&](const auto& r) {
					r->rebuild_layouts(context, r);
				});
			}
			
			template <class C>
			void draw(const C context) const {
				context(context.get_root_id(), [&](const auto& r) {
					r->draw_children(context, r);
				});
			}
		};
	}
}

