#pragma once
#include "../rect.h"

namespace db {
	namespace graphics {
		namespace gui {
			namespace controls {
				struct square_grid : public rect {
					square_grid(point pos, rect_wh square_size, int columns, int rows);
					void draw_proc(draw_info) override;
				};
			}
		}
	}
}