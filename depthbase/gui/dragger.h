#pragma once
#include "math/vec2.h"

namespace augs {
	namespace graphics {
		namespace gui {
			struct dragger {
				float vel[2], vel_mult;
				dragger();
				void stop();
				void move(vec2&);
				void drag(const vec2i& mouse, const rects::ltrb<float>&);
			};
		}
	}
}