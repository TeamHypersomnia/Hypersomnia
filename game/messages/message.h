#pragma once
#include "game/entity_id.h"

namespace messages {
	/* by default, all messages are considered to be of internal C++ framework use; 
	only by setting send_to_script flag in certain places in game do we note that scripts should be notified of this event.
	*/
	struct message {
		entity_id subject;
		bool delete_this_message = false;

		message(entity_id subject = entity_id()) : subject(subject) {}
	};
}