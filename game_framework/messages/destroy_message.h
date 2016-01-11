#pragma once
#include "message.h"

namespace messages {
	struct destroy_message : public message {
		bool only_children = false;
		destroy_message(augs::entity_id s) : message(s) {}
	};
}