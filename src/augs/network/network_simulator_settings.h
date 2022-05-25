#pragma once
#include <compare>
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

		bool operator==(const network_simulator_settings&) const = default;
	};

	using maybe_network_simulator = maybe<network_simulator_settings>;
}
