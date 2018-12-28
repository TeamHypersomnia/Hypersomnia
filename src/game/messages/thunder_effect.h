#pragma once
#include "game/messages/thunder_input.h"
#include "game/messages/message.h"

namespace messages {
	struct thunder_effect : predicted_message {
		using predicted_message::predicted_message;
		thunder_input payload;

		operator thunder_input() const {
			return payload;
		}
	};
}

