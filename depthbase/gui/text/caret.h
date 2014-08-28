#include "../system.h"

namespace db {
	namespace graphics {
		namespace gui {
			namespace text {
				struct caret_info {
					unsigned pos;
					int selection_offset;
					style default_style;

					unsigned get_left_selection() const;
					unsigned get_right_selection() const;
					caret_info(style);
				};
			}
		}
	}
}