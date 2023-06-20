#pragma once

enum class attitude_type {
	// GEN INTROSPECTOR enum class attitude_type
	NEUTRAL,
	WANTS_TO_HEAL,
	WANTS_TO_KILL,
	WANTS_TO_KNOCK_UNCONSCIOUS,
	COUNT
	// END GEN INTROSPECTOR
};

bool is_hostile(const attitude_type);