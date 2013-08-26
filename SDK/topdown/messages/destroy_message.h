#pragma once
#include "message.h"

namespace messages {
	struct destroy_message : public message {
		destroy_message(augmentations::entity_system:: entity* subject = nullptr) : message(subject) {}
	};
}