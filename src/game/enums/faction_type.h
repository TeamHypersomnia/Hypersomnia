#pragma once
#include "augs/graphics/rgba.h"

enum class faction_type {
	// GEN INTROSPECTOR enum class faction_type
	SPECTATOR,

	METROPOLIS,
	ATLANTIS,
	RESISTANCE,

	COUNT
	// END GEN INTROSPECTOR
};

namespace augs {
	template <class T, class _enum>
	class enum_array;
}

template <class T>
using per_faction_t = augs::enum_array<T, faction_type>;
