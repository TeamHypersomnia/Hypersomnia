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

	static bool is_actual_faction(const faction_type t)
	{
		return t == faction_type::METROPOLIS || t == faction_type::ATLANTIS || t == faction_type::RESISTANCE;
	}

	T& operator[](const faction_type t) {
		if (!is_actual_faction(t))
		{
			// Default to Atlantis
			return items[static_cast<int>(faction_type::ATLANTIS) - 1];
		}

		return items[static_cast<int>(t) - 1];
	}

	const T& operator[](const faction_type t) const {
		if (!is_actual_faction(t))
		{
			// Default to Atlantis
			return items[static_cast<int>(faction_type::ATLANTIS) - 1];
		}

		return items[static_cast<int>(t) - 1];
	}
};
