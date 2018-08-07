#pragma once
#include "augs/graphics/rgba.h"

enum class faction_type {
	// GEN INTROSPECTOR enum class faction_type
	NONE,

	METROPOLIS,
	ATLANTIS,
	RESISTANCE,

	COUNT
	// END GEN INTROSPECTOR
};

inline auto get_faction_color(const faction_type f) {
	switch (f) {
		case faction_type::NONE: return gray;
		case faction_type::METROPOLIS: return metropolis_color;
		case faction_type::ATLANTIS: return atlantis_color;
		case faction_type::RESISTANCE: return resistance_color;
		default: return white;
	}
}
