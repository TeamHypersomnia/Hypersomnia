#include "input_profiles.h"

namespace input_profiles {
	components::input_receiver crosshair() {
		components::input_receiver out;
		out.add(intent_type::MOVE_CROSSHAIR);
		out.add(intent_type::CROSSHAIR_PRIMARY_ACTION);
		out.add(intent_type::CROSSHAIR_SECONDARY_ACTION);
		return out;
	}

	components::input_receiver character() {
		components::input_receiver out;
		out.add(intent_type::PRESS_TRIGGER);
		out.add(intent_type::MOVE_BACKWARD);
		out.add(intent_type::MOVE_FORWARD);
		out.add(intent_type::MOVE_LEFT);
		out.add(intent_type::MOVE_RIGHT);
		out.add(intent_type::RELEASE_CAR);
		out.add(intent_type::HAND_BRAKE);
		return out;
	}
}