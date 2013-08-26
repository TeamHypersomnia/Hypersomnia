#pragma once
#include "entity_system/entity_ptr.h"

namespace messages {
	struct message {
		augmentations::entity_system::entity_ptr subject;
		message(augmentations::entity_system::entity* subject = nullptr) : subject(subject) {}
	};
}