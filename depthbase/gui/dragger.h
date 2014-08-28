#pragma once
#include "../../math/math.h"

namespace db {
	namespace graphics {
		namespace gui {
			struct dragger {
				float vel[2], vel_mult;
				dragger();
				void stop();
				void move(math::pointf&);
				void drag(const math::point& mouse, const math::rect_ltrb&);
			};
		}
	}
}