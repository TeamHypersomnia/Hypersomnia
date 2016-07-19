#include "draw_and_event_infos.h"

namespace augs {
	namespace gui {
		draw_info::draw_info(const rect_world& owner, vertex_triangle_buffer& v) : owner(owner), v(v) {}
		raw_event_info::raw_event_info(rect_world& owner, unsigned msg) : owner(owner), msg(msg), mouse_fetched(false), scroll_fetched(false) {}
		event_info::event_info(rect_world& owner, gui_event msg) : owner(owner), msg(msg) {}

		event_info::operator gui_event() {
			return msg;
		}

		event_info& event_info::operator=(gui_event m) {
			msg = m;
			return *this;
		}
	}
}