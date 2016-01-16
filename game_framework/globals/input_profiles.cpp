#include "input_profiles.h"

namespace input_profiles {
	components::input crosshair() {
		components::input out;
		out.add(messages::intent_message::MOVE_CROSSHAIR);
		out.add(messages::intent_message::CROSSHAIR_PRIMARY_ACTION);
		out.add(messages::intent_message::CROSSHAIR_SECONDARY_ACTION);
		return out;
	}

	components::input character() {
		components::input out;
		out.add(messages::intent_message::PRESS_TRIGGER);
		out.add(messages::intent_message::MOVE_BACKWARD);
		out.add(messages::intent_message::MOVE_FORWARD);
		out.add(messages::intent_message::MOVE_LEFT);
		out.add(messages::intent_message::MOVE_RIGHT);
		out.add(messages::intent_message::RELEASE_CAR);
		out.add(messages::intent_message::HAND_BRAKE);
		return out;
	}
}