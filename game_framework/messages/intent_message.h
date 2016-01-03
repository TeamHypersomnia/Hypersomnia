#pragma once
#include "message.h"
#include "math/vec2.h"

#include "window_framework/event.h"
#include "unmapped_intent_message.h"

/* everything is a state since for actions we can just ignore states with flag set to false */
using namespace augs;

namespace messages {
	struct intent_message : public unmapped_intent_message, public message {

	};
}