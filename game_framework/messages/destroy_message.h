#pragma once
#include "message.h"

namespace messages {
	struct destroy_message : public message {
		augs::entity_id redirection;
		bool only_children;
		destroy_message(augs::entity_id subject = augs::entity_id(), augs::entity_id redirection = augs::entity_id())
			: message(subject), only_children(false), redirection(redirection) {}
	};
}