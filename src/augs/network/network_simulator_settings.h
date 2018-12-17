#pragma once
#include "augs/templates/maybe.h"

namespace augs {
	struct network_simulator_settings {
		// GEN INTROSPECTOR struct augs::network_simulator_settings
		float latency_ms = 0.f;
		float jitter_ms = 0.f;
		float loss_percent = 0.f;
		float duplicates_percent = 0.f;
		// END GEN INTROSPECTOR

		static auto zero() {
			return network_simulator_settings();
		}

		bool operator==(const network_simulator_settings& b) const {
			return 
				latency_ms == b.latency_ms
				&& jitter_ms == b.jitter_ms
				&& loss_percent == b.loss_percent
				&& duplicates_percent == b.duplicates_percent
			;
		}

		bool operator!=(const network_simulator_settings& b) const {
			return !operator==(b);
		}
	};

	using maybe_network_simulator = maybe<network_simulator_settings>;
}
