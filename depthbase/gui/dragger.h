#pragma once
#include "math/vec2d.h"

namespace augs {
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