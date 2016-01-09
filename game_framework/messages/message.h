#pragma once
#include "entity_system/entity_id.h"

namespace messages {
	/* by default, all messages are considered to be of internal C++ framework use; 
	only by setting send_to_script flag in certain places in game_framework do we note that scripts should be notified of this event.
	*/
	struct message {
		augs::entity_id subject;
		message(augs::entity_id subject = augs::entity_id()) : subject(subject) {}
	};
}