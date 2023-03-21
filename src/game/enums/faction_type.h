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
	T metropolis;
	T atlantis;
	T resistance;
	// END GEN INTROSPECTOR

	T& operator[](const faction_type t) {
		switch (t) {
			case faction_type::METROPOLIS:
				return metropolis;
			case faction_type::RESISTANCE:
				return resistance;
			default:
				return atlantis;
		}
	}

	const T& operator[](const faction_type t) const {
		switch (t) {
			case faction_type::METROPOLIS:
				return metropolis;
			case faction_type::RESISTANCE:
				return resistance;
			default:
				return atlantis;
		}
	}
};
