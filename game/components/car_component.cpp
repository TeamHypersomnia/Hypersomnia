#include "car_component.h"

namespace components {
	void car::reset_movement_flags() {
		accelerating = decelerating = turning_left = turning_right = hand_brake = false;
	}
}