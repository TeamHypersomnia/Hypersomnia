#pragma once
#include "padding_byte.h"

namespace components {
	struct trigger_query_detector {
		// GEN INTROSPECTOR struct components::trigger_query_detector
		bool detection_intent_enabled = false;
		bool spam_trigger_requests_when_detection_intented = false;
		std::array<padding_byte, 2> pad;
		// END GEN INTROSPECTOR
	};
}