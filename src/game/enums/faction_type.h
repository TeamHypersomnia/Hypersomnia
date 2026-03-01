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
	ANY,
	COUNT
	// END GEN INTROSPECTOR
};

inline bool is_actual_faction(const faction_type f) {
	switch(f) {
		case faction_type::METROPOLIS:
		case faction_type::ATLANTIS:
		case faction_type::RESISTANCE:
			return true;
		default:
			return false;
	}
}

inline bool is_waypoint_for_faction(
	const faction_type waypoint_faction,
	const faction_type bot_faction
) {
	if (waypoint_faction == faction_type::ANY || waypoint_faction == faction_type::DEFAULT) {
		return true;
	}

	return waypoint_faction == bot_faction;
}

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

	template <class F>
	void for_each(F&& callback) {
		callback(metropolis);
		callback(atlantis);
		callback(resistance);
	}

	template <class F>
	void for_each(F&& callback) const {
		callback(metropolis);
		callback(atlantis);
		callback(resistance);
	}

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

	bool operator==(const per_actual_faction<T>&) const = default;
};
