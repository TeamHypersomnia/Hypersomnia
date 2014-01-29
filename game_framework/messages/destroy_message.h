#pragma once
#include "message.h"

namespace messages {
	struct destroy_message : public message {
		augmentations::entity_system::entity * redirection;
		bool only_children;
		destroy_message(augmentations::entity_system::entity* subject = nullptr, augmentations::entity_system::entity* redirection = nullptr) 
			: message(subject), only_children(false), redirection(redirection) {}
	};
}