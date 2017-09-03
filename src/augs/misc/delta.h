#pragma once
#include "augs/math/declare_math.h"

namespace augs {
	struct introspection_access;

	class delta {
		// GEN INTROSPECTOR class augs::delta
		real32 delta_ms;
		// END GEN INTROSPECTOR

		friend struct introspection_access;
	public:
		delta(const real32 delta_ms = static_cast<real32>(0))
			: delta_ms(delta_ms)
		{}

		bool operator==(const delta& b) const {
			return delta_ms == b.delta_ms;
		}

		float in_milliseconds() const;
		float in_seconds() const;
	};
}
