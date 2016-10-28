#pragma once
#include "augs/graphics/vertex.h"
#include "augs/window_framework/event.h"
#include "gui_event.h"

class rect_world;

namespace augs {
	namespace gui {
		struct draw_info {
			vertex_triangle_buffer& v;
			draw_info(vertex_triangle_buffer&);
		};

		struct event_traversal_flags {
			const augs::window::event::change change;
			
			bool was_hovered_rect_visited = false;
			bool mouse_fetched = false;
			bool scroll_fetched = false;
			event_traversal_flags(const augs::window::event::change);
		};

		struct event_info {
			gui_event msg;

			event_info(const gui_event);
			operator gui_event() const;
		};
	}
}
