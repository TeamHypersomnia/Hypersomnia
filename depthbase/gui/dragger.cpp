#pragma once
#include "dragger.h"
#include "../../window/timer.h"

namespace augs {
	using namespace math;
	namespace graphics {
		extern augs::window::fpstimer fps;

		namespace gui {
			dragger::dragger() : vel_mult(1.f) {
				stop();
			}

			void dragger::move(pointf& p) {
				p -= pointf(float(vel[0] * fps.frame_speed()), float(vel[1] * fps.frame_speed()));
			}

			void dragger::drag(const point& m, const rect_ltrb& rc) {
				stop();
				if(m.x < rc.l) vel[0] = float(-(rc.l - m.x));
				else if(m.x > rc.r) vel[0] = float(m.x - rc.r);
				if(m.y < rc.t) vel[1] = float(-(rc.t - m.y));
				else if(m.y > rc.b) vel[1] = float(m.y - rc.b);
				vel[0] *= -vel_mult;
				vel[1] *= -vel_mult;
			}

			void dragger::stop() {
				vel[0] = vel[1] = 0.f;
			}
		}
	}
}