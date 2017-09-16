#include "dragger.h"

namespace augs {
	namespace gui {
		dragger::dragger() : vel_mult(1.f) {
			stop();
		}

		void dragger::move(vec2& p, float delta) {
			p -= vec2(float(vel[0] * delta), float(vel[1] * delta));
		}

		void dragger::drag(const vec2i& m, const ltrb& rc) {
			stop();
			if (m.x < rc.l) vel[0] = float(-(rc.l - m.x));
			else if (m.x > rc.r) vel[0] = float(m.x - rc.r);
			if (m.y < rc.t) vel[1] = float(-(rc.t - m.y));
			else if (m.y > rc.b) vel[1] = float(m.y - rc.b);
			vel[0] *= -vel_mult;
			vel[1] *= -vel_mult;
		}

		void dragger::stop() {
			vel[0] = vel[1] = 0.f;
		}
	}
}