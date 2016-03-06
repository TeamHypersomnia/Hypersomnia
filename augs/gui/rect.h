#pragma once
#include <vector>
#include "material.h"
#include <limits>
#include "rect_id.h"

namespace augs {
	namespace gui {
		struct stylesheet;
		class gui_world;

		struct rect {
			enum class gui_event {
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
				gui_world& owner;
				std::vector<augs::vertex_triangle>& v;

				draw_info(gui_world&, std::vector<augs::vertex_triangle>&);
			};

			struct poll_info {
				gui_world& owner;
				const unsigned msg;

				bool mouse_fetched = false;
				bool scroll_fetched = false;
				poll_info(gui_world&, unsigned);
			};

			struct event_info {
				gui_world& owner;
				gui_event msg;

				event_info(gui_world&, gui_event);
				operator gui_event();
				event_info& operator=(gui_event);
			};

			rect_id next_focusable = nullptr;
			rect_id prev_focusable = nullptr;

			bool enable_drawing = true;
			bool enable_drawing_of_children = true;
			bool clip = true;
			bool fetch_wheel = false;
			bool scrollable = true;
			bool snap_scroll_to_content_size = true;
			bool preserve_focus = false;
			bool focusable = true;

			vec2i rc_pos_before_dragging;

			rects::ltrb<float> rc; /* actual rectangle */
			rects::wh<float> content_size; /* content's (children's) bounding box */
			vec2 scroll; /* scrolls content */

			std::vector<rect_id> children;

			rect(rects::xywh<float> rc = rects::xywh<float>());

			virtual rects::wh<float> get_content_size();

			void calculate_clipped_rectangle_layout();

			void consume_raw_input_and_generate_gui_events(poll_info&); /* event generator */

			virtual void consume_gui_event(event_info); /* event listener */
			virtual void draw_triangles(draw_info);

			/* consume_gui_event default subroutines */
			void scroll_content_with_wheel(event_info);
			void try_to_enable_middlescrolling(event_info);
			void try_to_make_this_rect_focused(event_info);

			/* focus switching routines */
			/* if this handler returns true, tab should not later be processed as a keydown event (used in textbox, for example) */
			bool focus_next_rect_by_tab(event_info);
			/* if this handler returns true, arrows should not later be processed as a keydown event (used in textbox, for example) */
			bool focus_next_rect_by_arrows(event_info);
			/* if this handler returns true, enter should not later be processed as a keydown event (used in textbox, for example) */
			bool focus_next_rect_by_enter(event_info);

			/* draw_triangles default subroutines */
			void draw_stretched_texture(draw_info in, const material& = material()),
				draw_centered_texture(draw_info in, const material& = material(), vec2i offset = vec2i()),
				draw_rectangle_stylesheeted(draw_info in, const stylesheet&),
				draw_children(draw_info in);

			virtual void get_member_children(std::vector<rect_id>& children);

			void get_all_descendants(std::vector<rect_id>&);

			/*  does scroll not exceed the content */
			bool is_scroll_clamped_to_right_down_corner();

			/* align scroll to not exceed the content */
			void clamp_scroll_to_right_down_corner(),
				/* try to scroll to view whole content */
				scroll_to_view();

			/* if last is nullptr, focus will be cycled on this rect (next = this) */
			/* not implemented */
			void gen_focus_links_depth(rect_id next = nullptr);
			void gen_focus_links();

			const rects::ltrb<float>& get_clipped_rect() const;
			rects::ltrb<float> get_rect_absolute() const;
			const vec2i& get_absolute_xy() const;
			rects::ltrb<float> get_local_clipper() const;
			rects::ltrb<float> get_clipping_rect() const;
			rect_id get_parent() const;

			static rect_id seek_focusable(rect_id, bool);

			void cache_descendants_before_children_reassignment();
		protected:
			friend class gui_world;

			rect_id parent = nullptr;
			std::vector<rect_id> cached_descendants;

			virtual void perform_logic_step(gui_world&);
		private:
			rects::ltrb<float> rc_clipped;
			rects::ltrb<float> clipping_rect = rects::ltrb<float>(0, 0, std::numeric_limits<int>::max() / 2, std::numeric_limits<int>::max() / 2);

			vec2i absolute_xy;

			bool was_hovered = false;
		};
	}
}