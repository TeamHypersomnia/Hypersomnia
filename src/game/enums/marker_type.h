#pragma once

enum class marker_letter_type {
	// GEN INTROSPECTOR enum class marker_letter_type
	A,
	B,
	C,
	D,

	COUNT
	// END GEN INTROSPECTOR
};

enum class point_marker_type {
	// GEN INTROSPECTOR enum class point_marker_type
	TEAM_SPAWN,
	FFA_SPAWN,
	PLAYTESTING_SPAWN,

	PORTAL_EXIT
	// END GEN INTROSPECTOR
};

enum class area_marker_type {
	// GEN INTROSPECTOR enum class area_marker_type
	BOMBSITE,
	BUY_ZONE,

	ORGANISM_AREA,

	PORTAL,

	CALLOUT,
	PREFAB
	// END GEN INTROSPECTOR
};

inline bool is_bombsite(const area_marker_type t) {
	return t == area_marker_type::BOMBSITE;
}
