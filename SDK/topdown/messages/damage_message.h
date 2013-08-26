#pragma once
#include "message.h"

namespace messages {
	struct damage_message : public message {
		float amount;
	};
}