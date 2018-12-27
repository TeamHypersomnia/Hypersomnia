#pragma once

enum class point_marker_type {
	// GEN INTROSPECTOR enum class point_marker_type
	TEAM_SPAWN,
	FFA_SPAWN,

	EQUIPMENT_GENERATOR
	// END GEN INTROSPECTOR
};

enum class area_marker_type {
	// GEN INTROSPECTOR enum class area_marker_type
	BOMBSITE_A,
	BOMBSITE_B,
	BOMBSITE_C,
	BUY_AREA,

	ORGANISM_AREA
	// END GEN INTROSPECTOR
};

inline bool is_bombsite(const area_marker_type t) {
	switch(t) {
		case area_marker_type::BOMBSITE_A: return true;
		case area_marker_type::BOMBSITE_B: return true;
		case area_marker_type::BOMBSITE_C: return true;
		default: return false;
	}
}
