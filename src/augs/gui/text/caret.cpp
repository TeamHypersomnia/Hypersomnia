#include "caret.h"

namespace augs {
	namespace gui {
		namespace text {
			caret_info::caret_info(const style default_style) : default_style(default_style) {}

			unsigned caret_info::get_left_selection() const {
				return selection_offset < 0 ? (pos + selection_offset) : pos;
			}

			unsigned caret_info::get_right_selection() const {
				return selection_offset < 0 ? pos : (pos + selection_offset);
			}
		}
	}
}
