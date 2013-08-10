#pragma once
#include "message.h"

namespace messages {
	struct moved_message : public message {
		/* new transform can be obtained from subject */
		moved_message(augmentations::entity_system::entity* subject) : message(subject) {}
	};
}