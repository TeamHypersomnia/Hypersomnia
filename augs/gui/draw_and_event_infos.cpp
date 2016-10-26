#include "draw_and_event_infos.h"

namespace augs {
	namespace gui {
		draw_info::draw_info(vertex_triangle_buffer& v) : v(v) {}
		raw_event_info::raw_event_info(const augs::window::event::state state) : state(state), mouse_fetched(false), scroll_fetched(false) {}
		event_info::event_info(const gui_event msg) : msg(msg) {}

		event_info::operator gui_event() const {
			return msg;
		}
	}
}