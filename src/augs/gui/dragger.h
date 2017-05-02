#pragma once
#include "augs/math/vec2.h"

namespace augs {
	namespace gui {
		struct dragger {
			float vel[2], vel_mult;
			dragger();
			void stop();
			void move(vec2&, float delta);
			void drag(const vec2i& mouse, const ltrb&);
		};
	}
}