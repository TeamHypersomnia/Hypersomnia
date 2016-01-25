#pragma once
#include <vector>
#include "quad.h"
#include <limits>

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
				
				struct draw_info {
					group& owner;
					std::vector<augs::vertex_triangle>& v;

					draw_info(group&, std::vector<augs::vertex_triangle>&); 
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

				rect* focus_next = nullptr;
				rect* focus_prev = nullptr;

				bool draw = true;
				bool clip = true;
				bool fetch_wheel = false;
				bool scrollable = true;
				bool snap_scroll_to_content = true;
				bool preserve_focus = false;
				bool focusable = true;

				vec2i drag_origin;
				rects::ltrb<float> rc; /* actual rectangle */ 
				rects::wh<float> content_size; /* content's (children's) bounding box */
				vec2 scroll; /* scrolls content */
				
				std::vector<rect*> children;
				
				rect(const rects::xywh<float>& rc = rects::xywh<float>());
				
				virtual rects::wh<float> get_content_size();
				
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
				/* if this handler returns true, enter should not later be processed as a keydown event (used in textbox, for example) */
				bool handle_enter(event_info);

				/* draw_proc default subroutines */
				void draw_rect		(draw_info in, const material& = material()), 
					 draw_rect		(draw_info in, const stylesheet&),
					 draw_children	(draw_info in);

				virtual void get_member_children(std::vector<rect*>& children);

				/* passes 0 or clipper's rc_clipped as clipper depending on clipper's clip flag */
				static rects::ltrb<float> add_quad (const material&, const rects::ltrb<float>& global, const rect* clipper, std::vector<augs::vertex_triangle>& v);
				
				/* simpler routine for more complex draws like text, origin is shifted to be local */
				rects::ltrb<float>		 local_add(const material&, const rects::ltrb<float>& local, std::vector<augs::vertex_triangle>& v) const;
				
				/*  does scroll not exceed the content */
				bool is_scroll_clamped_to_right_down_corner();

				/* align scroll to not exceed the content */
				void clamp_scroll_to_right_down_corner(), 
				/* try to scroll to view whole content */    
					scroll_to_view();  

				/* if last is nullptr, focus will be cycled on this rect (next = this) */
				/* not implemented */
				void gen_focus_links_depth(rect* next = nullptr);
				void gen_focus_links	  ();
				
				const rects::ltrb<float>& get_clipped_rect() const;
				rects::ltrb<float> get_rect_absolute() const;
				const vec2i& get_absolute_xy() const;
				rects::ltrb<float> get_local_clipper() const;
				rects::ltrb<float> get_clipping_rect() const;
				rect* get_parent() const;

				static rect* seek_focusable(rect*, bool);
			protected:
				friend class group;
				rect* parent = nullptr;
			private:
				rects::ltrb<float> rc_clipped;
				rects::ltrb<float> clipping_rect = rects::ltrb<float>(0, 0, std::numeric_limits<int>::max() / 2, std::numeric_limits<int>::max() / 2);
				
				vec2i absolute_xy;

				bool was_hovered = false;
			};
		}
	}
}