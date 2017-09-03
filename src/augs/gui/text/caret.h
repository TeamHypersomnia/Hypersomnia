#pragma once
#include "augs/gui/formatted_string.h"

namespace augs {
	namespace gui {
		namespace text {
			struct caret_info {
				unsigned pos = 0;
				int selection_offset = 0;
				style default_style;

				caret_info(style);

				unsigned get_left_selection() const;
				unsigned get_right_selection() const;
			};
		}
	}
}