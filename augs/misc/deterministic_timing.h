#pragma once

namespace augs {
	class world;

	struct deterministic_timeout {
		unsigned long long step_recorded = 0;
		bool was_set = false;
		float timeout_ms = 1000.f;

		void set(float timeout_ms);
		deterministic_timeout(float timeout_ms);
	};
}