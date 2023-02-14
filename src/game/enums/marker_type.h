#pragma once

enum class marker_letter_type {
	// GEN INTROSPECTOR enum class marker_letter_type
	A,
	B,
	C,

	COUNT
	// END GEN INTROSPECTOR
};

enum class point_marker_type {
	// GEN INTROSPECTOR enum class point_marker_type
	TEAM_SPAWN,
	FFA_SPAWN,
	PLAYTESTING_SPAWN
	// END GEN INTROSPECTOR
};

enum class area_marker_type {
	// GEN INTROSPECTOR enum class area_marker_type
	BOMBSITE,
	BOMBSITE_DUMMY_B,
	BOMBSITE_DUMMY_C,
	BUY_AREA,

	ORGANISM_AREA,

	CALLOUT
	// END GEN INTROSPECTOR
};

inline bool is_bombsite(const area_marker_type t) {
	switch(t) {
		case area_marker_type::BOMBSITE: return true;
		case area_marker_type::BOMBSITE_DUMMY_B: return true;
		case area_marker_type::BOMBSITE_DUMMY_C: return true;
		default: return false;
	}
}
