#pragma once

namespace augs {
	struct stepped_timestamp;

	class delta {
		float delta_ms = 0;
	public:
		delta(const float dt = 0.f) : delta_ms(dt) { }

		bool operator==(const delta& b) const {
			return delta_ms == b.delta_ms;
		}

		float in_milliseconds() const;
		float in_seconds() const;
	};
}
