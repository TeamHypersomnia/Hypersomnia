#include "gui_traversal_structs.h"

namespace augs {
	namespace gui {
		draw_info::draw_info(vertex_triangle_buffer& v) : v(v) {}
		event_traversal_flags::event_traversal_flags(const augs::window::event::change change) : change(change) {}
		event_info::event_info(const gui_event msg) : msg(msg) {}

		event_info::operator gui_event() const {
			return msg;
		}
	}
}