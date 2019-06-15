#pragma once
#include "augs/graphics/rgba.h"
#include "augs/misc/enum/enum_array.h"

enum class faction_type {
	// GEN INTROSPECTOR enum class faction_type
	SPECTATOR,

	METROPOLIS,
	ATLANTIS,
	RESISTANCE,

	DEFAULT,
	COUNT
	// END GEN INTROSPECTOR
};

namespace augs {
	template <class T, class _enum>
	class enum_array;
}

template <class T>
using per_faction_t = augs::enum_array<T, faction_type>;

template <class T>
struct per_actual_faction {
	// GEN INTROSPECTOR struct per_actual_faction class T
	std::array<T, 3> items;
	// END GEN INTROSPECTOR

	T& operator[](const faction_type t) {
		return items[static_cast<int>(t) - 1];
	}

	const T& operator[](const faction_type t) const {
		return items[static_cast<int>(t) - 1];
	}
};
