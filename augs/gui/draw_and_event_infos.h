#pragma once
#include "augs/graphics/vertex.h"
#include "augs/window_framework/event.h"
#include "gui_event.h"

class rect_world;

namespace augs {
	namespace gui {
		struct draw_info {
			const rect_world& owner;
			vertex_triangle_buffer& v;

			draw_info(const rect_world&, vertex_triangle_buffer&);
		};

		struct raw_event_info {
			rect_world& owner;
			const augs::window::event::message msg;

			bool mouse_fetched = false;
			bool scroll_fetched = false;
			raw_event_info(rect_world&, const augs::window::event::message);
		};

		struct event_info {
			rect_world& owner;
			gui_event msg;

			event_info(rect_world&, const gui_event);
			operator gui_event() const;
		};
	}
}
