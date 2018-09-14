#pragma once
#include "game/messages/message.h"
#include "game/enums/battle_event.h"

namespace messages {
	struct battle_event_message : message {
		battle_event event;
	};
}
