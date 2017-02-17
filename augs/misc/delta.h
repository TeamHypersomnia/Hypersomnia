#pragma once

namespace augs {
	class delta {
		float delta_ms;
	public:
		delta(const float delta_ms = 0.f) 
			: delta_ms(delta_ms)
		{}

		bool operator==(const delta& b) const {
			return delta_ms == b.delta_ms;
		}

		float in_milliseconds() const;
		float in_seconds() const;
	};
}
