#pragma once
#include "message.h"

namespace messages {
	struct destroy_message : public message {
		augs::entity_system::entity * redirection;
		bool only_children;
		destroy_message(augs::entity_system::entity* subject = nullptr, augs::entity_system::entity* redirection = nullptr) 
			: message(subject), only_children(false), redirection(redirection) {}
	};
}