#pragma once

namespace components {
	struct trigger_query_detector {
		bool detection_intent_enabled = false;
		bool spam_trigger_requests_when_detection_intented = false;
	};
}