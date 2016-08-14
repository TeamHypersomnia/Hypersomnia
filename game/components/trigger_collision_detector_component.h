#pragma once

namespace components {
	struct trigger_collision_detector {
		int detection_intent_enabled = false;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(
				CEREAL_NVP(detection_intent_enabled)
			);
		}
	};
}