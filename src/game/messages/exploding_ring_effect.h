#pragma once
#include "game/messages/exploding_ring_input.h"
#include "game/messages/message.h"

namespace messages {
	struct exploding_ring_effect : predicted_message {
		using predicted_message::predicted_message;
		exploding_ring_input payload;

		operator exploding_ring_input() const {
			return payload;
		}
	};
}
