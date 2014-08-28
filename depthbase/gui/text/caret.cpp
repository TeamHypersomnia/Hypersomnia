#include "caret.h"

namespace db {
	namespace graphics {
		namespace gui {
			namespace text {
				caret_info::caret_info(style default_style) : default_style(default_style), pos(0), selection_offset(0) {}
				
				unsigned caret_info::get_left_selection() const {
					return selection_offset < 0 ? (pos + selection_offset) : pos;
				}

				unsigned caret_info::get_right_selection() const {
					return selection_offset < 0 ? pos : (pos + selection_offset);
				}
			}
		}
	}
}
