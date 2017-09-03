#include "gui_traversal_structs.h"

namespace augs {
	namespace gui {
		raw_input_traversal::raw_input_traversal(const augs::event::change change) : change(change) {}
		event_info::event_info(const gui_event msg, const int scroll_amount, vec2i total_dragged_amount) : msg(msg), scroll_amount(scroll_amount), total_dragged_amount(total_dragged_amount) {}
		
		bool event_info::is_ldown_or_double_or_triple() const {
			return msg == gui_event::ldown || msg == gui_event::ldoubleclick || msg == gui_event::ltripleclick;
		}

		event_info::operator gui_event() const {
			return msg;
		}
	}
}