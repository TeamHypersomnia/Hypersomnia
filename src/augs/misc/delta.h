#pragma once

namespace augs {
	struct introspection_access;

	class delta {
		// GEN INTROSPECTOR class augs::delta
		float delta_ms;
		// END GEN INTROSPECTOR

		friend struct introspection_access;
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
