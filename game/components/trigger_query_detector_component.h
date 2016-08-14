#pragma once
#include "padding_byte.h"

namespace components {
	struct trigger_query_detector {
		bool detection_intent_enabled = false;
		bool spam_trigger_requests_when_detection_intented = false;
		padding_byte pad[2];

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(detection_intent_enabled),
				CEREAL_NVP(spam_trigger_requests_when_detection_intented)
			);
		}
	};
}