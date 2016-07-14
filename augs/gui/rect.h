#pragma once
#include <vector>
#include "material.h"
#include "rect_id.h"

namespace augs {
	namespace gui {
		struct stylesheet;
		class rect_world;

		struct rect {
			bool disable_hovering = false;
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
			rect(assets::texture_id);

			/* consume_gui_event default subroutines */
			void scroll_content_with_wheel(event_info);
			void try_to_enable_middlescrolling(event_info);
			void try_to_make_this_rect_focused(event_info);

			/* draw_triangles default subroutines */
			void draw_stretched_texture(draw_info in, const material& = material()) const,
				draw_centered_texture(draw_info in, const material& = material(), vec2i offset = vec2i()) const,
				draw_rectangle_stylesheeted(draw_info in, const stylesheet&) const,
				draw_children(draw_info in) const;

			/*  does scroll not exceed the content */
			bool is_scroll_clamped_to_right_down_corner();

			/* align scroll to not exceed the content */
			void clamp_scroll_to_right_down_corner(),
				/* try to scroll to view whole content */
				scroll_to_view();

			bool is_being_dragged(rect_world&);

			const rects::ltrb<float>& get_clipped_rect() const;
			rects::ltrb<float> get_rect_absolute() const;
			const vec2i& get_absolute_xy() const;
			rects::ltrb<float> get_local_clipper() const;
			rects::ltrb<float> get_clipping_rect() const;

			rect_id parent;

			rects::ltrb<float> rc_clipped;
			rects::ltrb<float> clipping_rect = rects::ltrb<float>(0.f, 0.f, std::numeric_limits<int>::max() / 2.f, std::numeric_limits<int>::max() / 2.f);

			vec2i absolute_xy;
		};
	}
}