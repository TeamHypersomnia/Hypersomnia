#pragma once
#include "game/cosmos/entity_id.h"

namespace messages {
	struct message {
		entity_id subject;

		message(const entity_id subject = {}) : subject(subject) {}
	};
}