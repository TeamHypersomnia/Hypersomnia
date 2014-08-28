#pragma once
#include <vector>
#include "quad.h"

namespace augs {
	namespace graphics {
		namespace gui {
			struct stylesheet;
			class group;
			struct rect {
				enum class event {
					unknown,
					keydown,
					keyup,
					character,
					unichar,
					wheel,

					lpressed,
					rpressed,
					ldown,
					mdown,
					rdown,
					lclick,
					rclick,
					ldoubleclick,
					ltripleclick,
					mdoubleclick,
					rdoubleclick,
					lup,
					mup,
					rup,
					loutup,
					routup,
					hover,
					hout,
					ldrag,
					loutdrag,

					focus,
					blur
				};
				
				/* 
				index_info structure tells the index of a particular rectangle in the quad vector.

				If any child class overrides draw_proc to draw additional content, 
				it should derive index_info and quad_indices at the same names and add its own members.
				
				-1 means "not drawn" and "not in the vector"
				*/

				struct index_info {
					int background;
				} quad_indices;

				struct draw_info {
					group& owner;
					std::vector<quad>& v;

					draw_info(group&, std::vector<quad>&); 
				};

				struct poll_info {
					group& owner;
					const unsigned msg;

					bool mouse_fetched;
					bool scroll_fetched;
					poll_info(group&, unsigned); 
				};

				struct event_info {
					group& owner;
					event msg;

					event_info(group&, event);
					operator event();
					event_info& operator=(event);
				};

				enum class appearance {
					released,
					hovered,
					pushed,
					unknown
				};

				rect* focus_next, *focus_prev;

				bool draw, clip, fetch_wheel, scrollable, snap_scroll_to_content, preserve_focus, focusable;
				point drag_origin;
				rect_ltrb rc; /* actual rectangle */ 
				rect_wh content_size; /* content's (children's) bounding box */
				pointf scroll; /* scrolls content */
				
				std::vector<rect*> children;
				
				rect(const math::rect_xywh& rc = math::rect_xywh());
				
				virtual rect_wh get_content_size();
				
				void update_rectangles();
				virtual void update_proc(group&);

				virtual void poll_message(poll_info&); /* event generator */
				virtual void event_proc(event_info); /* event listener */
				virtual void draw_proc(draw_info);

				/* event_proc default subroutines */
				void handle_scroll(event_info);
				void handle_middleclick(event_info);
				void handle_focus(event_info);

				/* focus switching routines */
				/* if this handler returns true, tab should not later be processed as a keydown event (used in textbox, for example) */
				bool handle_tab(event_info);
				/* if this handler returns true, arrows should not later be processed as a keydown event (used in textbox, for example) */
				bool handle_arrows(event_info);
				/* if this handler returns true, arrows should not later be processed as a keydown event (used in textbox, for example) */
				bool handle_enter(event_info);

				/* draw_proc default subroutines */
				void draw_rect		(draw_info in, const material& = material()), 
					 draw_rect		(draw_info in, const stylesheet&),
					 draw_children	(draw_info in);

				virtual void get_member_children(std::vector<rect*>& children);

				/* passes 0 or clipper's rc_clipped as clipper depending on clipper's clip flag */
				static rect_ltrb add_quad (const material&, const rect_ltrb& global, const rect* clipper, std::vector<quad>& v);
				
				/* simpler routine for more complex draws like text, origin is shifted to be local */
				rect_ltrb		 local_add(const material&, const rect_ltrb& local, std::vector<quad>& v) const;
				
				/* how should rect look like depending on incoming event */
				static appearance get_appearance(event m); 
				
				/*  does scroll not exceed the content */
				bool is_scroll_aligned();

				/* align scroll to not exceed the content */
				void align_scroll(), 
				/* try to scroll to view whole content */    
					scroll_to_view();  


				/* if last is nullptr, focus will be cycled on this rect (next = this) */
				/* not implemented */
				void gen_focus_links_depth(rect* next = nullptr);
				void gen_focus_links	  ();
				
				const rect_ltrb& get_clipped_rect() const;
				rect_ltrb get_rect_absolute() const;
				const point& get_absolute_xy() const;
				rect_ltrb get_local_clipper() const;
				rect_ltrb get_clipping_rect() const;
				rect* get_parent() const;

				static rect* seek_focusable(rect*, bool);
			protected:
				friend class group;
				rect* parent; 
			private:
				rect_ltrb rc_clipped, clipping_rect;
				point absolute_xy;

				bool was_hovered;
			};
		}
	}
}